/*
 * RenderEngine.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "RenderEngine.h"
#include "cmsis_os.h"

void RenderEngineTask(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}