/*
 * InputHandler.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "InputHandler.h"
#include "cmsis_os.h"

void InputHandlerTask(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}