#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>

#include "mt3620_rdb.h"

#include "../SoftPWM/Inc/Public/SoftPWM.h"

// This C application for the MT3620 Reference Development Board (Azure Sphere)
// outputs a string every second to Visual Studio's Device Output window
//
// It uses the API for the following Azure Sphere application libraries:
// - log (messages shown in Visual Studio's Device Output window during debugging)

static volatile sig_atomic_t terminationRequested = false;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async signal safe
    terminationRequested = true;
}

/// <summary>
///     Main entry point for this sample.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("Application starting\n");

    // Register a SIGTERM handler for termination requests
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

	struct SOFTPWM_Config pwm0cfg = { .gpio = 0,.period = 20000000,.active = 500000 };
	struct SOFTPWM_Config pwm1cfg = { .gpio = 1,.period = 20000000,.active = 600000 };
	struct SOFTPWM_Config pwm2cfg = { .gpio = 2,.period = 40000000,.active = 700000 };

	struct _SOFTPWM_State *pwm0, *pwm1, *pwm2;

	SOFTPWM_Init(&pwm0cfg, &pwm0);
	SOFTPWM_Init(&pwm1cfg, &pwm1);
	SOFTPWM_Init(&pwm2cfg, &pwm2);
	
	if ((NULL == pwm0)|| (NULL == pwm1)|| (NULL == pwm2))
	{
		Log_Debug("PWM init error\n");
		return -1;
	}

	SOFTPWM_Start(pwm0);
	SOFTPWM_Start(pwm1);
	SOFTPWM_Start(pwm2);

    // Main loop
    const struct timespec sleepTime = {1, 0};
    while (!terminationRequested) {
        Log_Debug("Hello world\n");
        nanosleep(&sleepTime, NULL);
    }

	SOFTPWM_Destroy(pwm0);
	SOFTPWM_Destroy(pwm1);
	SOFTPWM_Destroy(pwm2);

    Log_Debug("Application exiting\n");
    return 0;
}
