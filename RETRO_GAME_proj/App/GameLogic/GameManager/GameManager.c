/*
 * GameManager.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "GameManager.h"
#include "cmsis_os.h"

void GameManagerTask(void *pvParameters)
{
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}