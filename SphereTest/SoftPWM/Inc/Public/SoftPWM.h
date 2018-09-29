#ifndef SOFTPWM_H
#define SOFTPWM_H

#include "applibs/gpio.h"

struct SOFTPWM_Config
{
	GPIO_Id gpio;
	int period;
	int active;
};

struct _SOFTPWM_State;

extern int SOFTPWM_Init(struct SOFTPWM_Config* config, struct _SOFTPWM_State** state);
extern int SOFTPWM_Destroy(struct _SOFTPWM_State* softPWMState);

extern int SOFTPWM_Start(struct _SOFTPWM_State* softPWMState);
extern int SOFTPWM_Stop(struct _SOFTPWM_State* softPWMState);

extern int SOFTPWM_SetPeriod(struct _SOFTPWM_State* softPWMState, int period, int active);

#endif
