/*
 * SystemManager.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "SystemManager.h"
#include "synchronization.h"
#include "systemDefs.h"
#include "cmsis_os.h"

static sys_state_t currentState = SYS_BOOT;

void SystemManagerTask(void *pvParameters)
{
    event_id_t receivedEvent;
    
    for(;;)
    {
        // Esperar eventos en qSystem
        if (xQueueReceive(qSystem, &receivedEvent, portMAX_DELAY) == pdTRUE) {
            
            switch (receivedEvent) {
                case SE_BOOT_COMPLETE:
                    currentState = SYS_MAIN_MENU;
                    // Aquí se enviarían eventos a UI para mostrar menú
                    event_id_t newEvent = SE_SHOW_MENU;
                    xQueueSend(qEvents, &newEvent, 0);
                    break;
                    
                case SE_START_NEW_GAME:
                    currentState = SYS_IN_GAME;
                    // Enviar eventos de inicio de juego
                    break;
                    
                case SE_PAUSE_GAME:
                    currentState = SYS_PAUSE;
                    break;
                    
                case SE_RESUME_GAME:
                    currentState = SYS_IN_GAME;
                    break;
                    
                case SE_EXIT_GAME:
                    currentState = SYS_MAIN_MENU;
                    break;
                    
                default:
                    // Evento no manejado - silencioso
                    break;
            }
        }
    }
}