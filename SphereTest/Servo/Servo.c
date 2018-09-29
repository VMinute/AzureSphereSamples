#include <Servo.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "../SoftPWM/Inc/Public/SoftPWM.h"

struct _SERVO_State
{
	struct _SOFTPWM_State* pwmState;
	int		minAngle;
	int		minPulse;
	int		period;
	double	nsPerDegree;
	int		curAngle;
};

int SERVO_Init(struct SERVO_Config* config, struct _SERVO_State** state)
{
	if ((NULL == config) || (NULL == state))
	{
		errno = EINVAL;
		goto error;
	}

	if ((config->maxAngle <= config->minAngle) ||
		(config->maxPulse <= config->minPulse) ||
		(config->period <= config->maxPulse))
	{
		errno = EINVAL;
		goto error;
	}

	*state = (struct _SERVO_State*)malloc(sizeof(struct _SERVO_State));

	(*state)->minPulse = config->minPulse;
	(*state)->nsPerDegree = ((double)(config->maxPulse - config->minPulse)) / (config->maxAngle - config->minAngle);
	(*state)->minAngle = (*state)->curAngle = config->minAngle;
	(*state)->period = config->period;
	(*state)->pwmState = NULL;

	struct SOFTPWM_Config pwmConfig;

	pwmConfig.gpio = config->gpio;
	pwmConfig.active = config->minPulse;
	pwmConfig.period = config->period;
	
	if (SOFTPWM_Init(&pwmConfig, &((*state)->pwmState)) < 0)
		goto error;

	if (SOFTPWM_Start((*state)->pwmState) < 0)
		goto error;

	return 0;

error:

	if (state)
	{
		if (*state)
			SERVO_Destroy(*state);

		*state = NULL;
	}
	return -1;
}

int SERVO_Destroy(struct _SERVO_State* servoState)
{
	if (NULL != servoState)
	{
		if (NULL!= servoState->pwmState)
			SOFTPWM_Destroy(servoState->pwmState);
		free(servoState);
	}
	return 0;
}

int SERVO_SetAngle(struct _SERVO_State* servoState, int angle)
{
	if (NULL == servoState)
	{
		errno = EINVAL;
		return -1;
	}

	int angleDiff = angle - servoState->curAngle;

	if (angleDiff == 0)
		return 0;

	struct	timespec ts;
	int		sign = (angle > servoState->curAngle) ? 1 : -1;

	ts.tv_sec = 0;
	ts.tv_nsec = servoState->period;

	for (int i = 0; i < abs(angleDiff); i++)
	{
		servoState->curAngle += sign;
		SOFTPWM_SetPeriod(servoState->pwmState, servoState->period, servoState->minPulse + (int)((servoState->curAngle - servoState->minAngle)*servoState->nsPerDegree));
		nanosleep(&ts, NULL);
	}
	return 0;
}