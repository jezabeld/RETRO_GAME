/*
 * GraphEngine.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "GraphEngine.h"
#include "synchronization.h"
#include "cmsis_os.h"
#include "ScreenDrv.h"
#include "lvgl.h"
#include "lv_port_disp.h" /* <- your display driver registration function   */
#include "lv_port_indev.h" /* <- your input device driver registration function */

extern tft_t myTft;
volatile UBaseType_t uxStackGFXTask;

void GraphEngineTask(void *pvParameters)
{
    // Fase 1: Inicializar LVGL
	lv_init();
    lv_port_disp_init(); /* <-- registra display */
    lv_port_indev_init(); /* <-- registra input devices */

    // Señalizar que GraphEngine está listo 
    xSemaphoreGive(semGFXReady);
    
#ifdef TEST_MODE
    // Enviar evento para FlowPlans indicando que LVGL está inicializado
    event_id_t gfxInitEvent = SE_GFX_INIT;
    xQueueSend(qEvents, &gfxInitEvent, 0);
#endif
    
    // Esperar que UI esté lista
    uxStackGFXTask = uxTaskGetStackHighWaterMark(NULL);
    xSemaphoreTake(semUiReady, portMAX_DELAY);
    
    // Fase 2: Loop principal LVGL con timing exacto
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20 FPS exactos
    
    for(;;)
    {
#ifdef TEST_MODE
        // Enviar evento SE_GFX_RUNNING para indicar actividad (solo para test)
        event_id_t gfxEvent = SE_GFX_RUNNING;
        xQueueSend(qEvents, &gfxEvent, 0);
#endif
        
        lv_timer_handler(); /* procesa LVGL */
		lv_port_indev_clear_buttons(); /* clear button states after processing */

		uxStackGFXTask = uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&xLastWakeTime, xFrequency); // Timing exacto cada 50ms
    }
}
