#include <SoftPWM.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <applibs/gpio.h>

// pulse is around 70us longer
#define LATENCY_ADJUSTMENT	70000

struct _SOFTPWM_State
{
	int enabled;
	int gpioFd;
	int period;
	int active;
	pthread_t thread;
};

static void SOFTPWM_Thread(volatile struct _SOFTPWM_State* state)
{
	struct timespec activeTS;
	struct timespec inactiveTS;
	struct timespec start, now;

	activeTS.tv_sec = 0;	
	inactiveTS.tv_sec = 0;

	while (state->enabled)
	{
		activeTS.tv_nsec = state->active- LATENCY_ADJUSTMENT;

		GPIO_SetValue(state->gpioFd, GPIO_Value_High);
		clock_gettime(CLOCK_MONOTONIC, &start);
		nanosleep(&activeTS, NULL);
		GPIO_SetValue(state->gpioFd, GPIO_Value_Low);
		clock_gettime(CLOCK_MONOTONIC, &now);
		inactiveTS.tv_nsec = state->period - (now.tv_nsec - start.tv_nsec);
		nanosleep(&inactiveTS, NULL);		
	}
}

int SOFTPWM_Init(struct SOFTPWM_Config* config, struct _SOFTPWM_State** state)
{
	if (!state)
	{
		errno = EINVAL;
		goto error;
	}

	*state = NULL;

	int gpioFd = -1;
	
	if (NULL == config)
	{
		errno = EINVAL;
		goto error;
	}

	if (config->period==0)
	{
		errno = EINVAL;
		goto error;
	}

	if (config->period < config->active)
	{
		errno = EINVAL;
		goto error;
	}

	// initializes GPIO pin
	gpioFd=GPIO_OpenAsOutput(config->gpio, GPIO_OutputMode_PushPull, GPIO_Value_Low);

	if (gpioFd < 0)
		goto error;

	(*state) = malloc(sizeof(struct _SOFTPWM_State));

	if (NULL == (*state))
	{
		errno = ENOMEM;
		goto error;
	}
	
	memset((*state), 0, sizeof(struct _SOFTPWM_State));

	(*state)->gpioFd = gpioFd;
	(*state)->period = config->period;
	(*state)->active = config->active;
	return 0;

error:

	if (gpioFd>=0)
		close(gpioFd);

	if (state && (*state))
		free(*state);

	return -1;
}

int SOFTPWM_Start(struct _SOFTPWM_State* softPWMState)
{
	if (NULL == softPWMState)
	{
		errno = EINVAL;
		return -1;
	}

	if (NULL != softPWMState->thread)
	{
		return 0;
	}

	softPWMState->enabled = 1;
	return pthread_create(&softPWMState->thread, NULL, (void * (*)(void *))SOFTPWM_Thread, softPWMState);
}

int SOFTPWM_Stop(struct _SOFTPWM_State* softPWMState)
{
	if (NULL == softPWMState)
	{
		errno = EINVAL;
		return -1;
	}

	if (NULL == softPWMState->thread)
		return 0;

	softPWMState->enabled = 0;
	return pthread_join(softPWMState->thread, NULL);
}

int SOFTPWM_SetPeriod(struct _SOFTPWM_State* softPWMState, int period, int active)
{
	if (NULL == softPWMState)
	{
		errno = EINVAL;
		return -1;
	}

	if (active<= LATENCY_ADJUSTMENT)
	{
		errno = EINVAL;
		return -1;
	}

	softPWMState->period = period;
	softPWMState->active = active;
	return 0;
}

int SOFTPWM_Destroy(struct _SOFTPWM_State* softPWMState)
{
	if (NULL == softPWMState)
	{
		errno = EINVAL;
		return -1;
	}

	SOFTPWM_Stop(softPWMState);
	free(softPWMState);
	return 0;
}
