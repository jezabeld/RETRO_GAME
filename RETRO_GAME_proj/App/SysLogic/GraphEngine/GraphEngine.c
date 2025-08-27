/*
 * GraphEngine.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "GraphEngine.h"
#include "events.h"
#include "cmsis_os.h"

void GraphEngineTask(void *pvParameters)
{
    for(;;)
    {
        // Enviar evento SE_GFX_RUNNING para indicar actividad
        event_id_t gfxEvent = SE_GFX_RUNNING;
        xQueueSend(qEvents, &gfxEvent, 0);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}