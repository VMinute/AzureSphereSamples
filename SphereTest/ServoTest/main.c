#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include <applibs/log.h>

#include "mt3620_rdb.h"

#include "../Servo/Inc/Public/Servo.h"

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

	struct SERVO_Config servoCfg;
	struct _SERVO_State* servo0;

	servoCfg.gpio = MT3620_GPIO0;
	servoCfg.minAngle = 0;
	servoCfg.maxAngle = 180;
	servoCfg.minPulse = 600000;
	servoCfg.maxPulse = 2400000;
	servoCfg.period = 20000000;

	if (SERVO_Init(&servoCfg, &servo0) < 0)
	{
		Log_Debug("Error initializing servo 0\n");
		return -1;
	}

	struct timespec sleepTime = { 1, 0 };

	// places servo at angle 0
	SERVO_SetAngle(servo0, 0);

	nanosleep(&sleepTime, NULL);

    // Main loop
	int angle = 0;
	int step = 1;

    while (!terminationRequested) {
        Log_Debug("Angle %d\n",angle);
		SERVO_SetAngle(servo0, angle);

		angle += step;

		if (angle == 180)
			step = -1;
		
		if (angle == 0)
			step = 1;
    }

    Log_Debug("Application exiting\n");
    return 0;
}
