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

/* Normalización helper: ADC 0..4095 -> -100..+100 */
static int16_t norm_adc_to_100(uint16_t adc)
{
    if (adc > 4095) adc = 4095;
    int32_t v = ((int32_t)adc * 200) / 4095 - 100;
    if (v < -100) v = -100;
    if (v > 100)  v = 100;
    return (int16_t)v;
}

void InputHandlerTask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
    for(;;)
    {
    	// Leer HW
		joystick_t joy_data;
		uint16_t adc_pitch = 0;
		uint16_t adc_roll = 0;
		
		if (inputGetJoyAxis(0, &joy_data) == 0) {  // X axis
			adc_roll = joy_data.jyX;
		}
		if (inputGetJoyAxis(1, &joy_data) == 0) {  // Y axis
			adc_pitch = joy_data.jyY;
		}

		uint8_t  pause_btn = /* btn PAUSE edge*/ 0;

		gameAction_t act = {
			.pitch = norm_adc_to_100(adc_pitch),
			.roll  = norm_adc_to_100(adc_roll),
			.pause = pause_btn,
		};

		(void)xQueueSend(qActions, &act, 0);
#ifdef TEST_MODE
		sendEvent(SE_INH_ACTION_SENT);
#endif
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(GFX_TICK_MS)); // Timing exacto cada 50ms
    }
}
