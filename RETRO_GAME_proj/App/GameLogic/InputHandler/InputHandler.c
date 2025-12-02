/*
 * InputHandler.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "InputHandler.h"
#include "GameManager.h"
#include "synchronization.h"
#include "cmsis_os.h"
#include "systemDefs.h"
#include "InputDrv.h"
#include "AccelDrv.h"

void InputHandlerTask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
    for(;;)
    {
    	// Leer acelerómetro (valores ya normalizados a -1024..+1023)
		accelTilt_t tilt;
		accelDrvGetTilt(&tilt);

		uint8_t  pause_btn = /* btn PAUSE edge*/ 0;

		gameAction_t act = {
			.pitch = tilt.tiltX,  // Valores normalizados desde AccelDrv (-1024..+1023)
			.roll  = tilt.tiltY,  // Valores normalizados desde AccelDrv (-1024..+1023)
			.pause = pause_btn,
		};

		(void)xQueueSend(qActions, &act, 0);
#if DEBUG_LEVEL >= 2
		sendEvent(SE_INH_ACTION_SENT);
#endif
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(GFX_TICK_MS)); // Timing exacto cada 50ms
    }
}
