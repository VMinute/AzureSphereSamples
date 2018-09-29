#ifndef SERVO_H
#define SERVO_H

#include "applibs/gpio.h"

struct SERVO_Config
{
	GPIO_Id gpio;
	int minAngle;
	int maxAngle;
	int period;
	int minPulse;
	int maxPulse;
};

struct _SERVO_State;

extern int SERVO_Init(struct SERVO_Config* config, struct _SERVO_State** state);
extern int SERVO_Destroy(struct _SERVO_State* servoState);

extern int SERVO_SetAngle(struct _SERVO_State* servoState,int angle);

#endif
