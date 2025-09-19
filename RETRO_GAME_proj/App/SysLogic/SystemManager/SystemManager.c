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
#include "GraphEngine.h"

extern TaskHandle_t tskGameMng;
extern TaskHandle_t tskRender;
extern TaskHandle_t tskInpHnd;
extern TaskHandle_t tskGraph;

/* Estados de la FSM principal del sistema */
typedef enum {
    SYS_SPLASH = 0,     // Pantalla inicial
    SYS_MAIN_MENU,      // Menú principal
    SYS_IN_GAME,        // Ejecutando gameplay
    SYS_PAUSE           // Menú de pausa
} sys_state_t;

static sys_state_t currentState = SYS_SPLASH;

static void wakeGameTasks(void);

void SystemManagerTask(void *pvParameters)
{
    event_id_t receivedEvent;
    
    for(;;)
    {
        // Esperar eventos en qSystem
        if (xQueueReceive(qSystem, &receivedEvent, portMAX_DELAY) == pdTRUE) {
            
            switch (receivedEvent) {
                case SE_BOOT_COMPLETE:
                    // Transición válida: SYS_SPLASH → SYS_MAIN_MENU
                    if (currentState == SYS_SPLASH) {
                        currentState = SYS_MAIN_MENU;
                        sendEvent(SE_SHOW_MENU);
                    }
                    break;
                    
                case SE_START_NEW_GAME:
                    // Transición válida: SYS_MAIN_MENU → SYS_IN_GAME
                    if (currentState == SYS_MAIN_MENU) {
                        currentState = SYS_IN_GAME;
                        // eventos para switch entre menu y juego
                        sendEvent(SE_HIDE_MENU); 		// para UICtrl
                        sendEvent(GE_START_NEW_GAME); 	// para GmeMng
                        xTaskNotify(tskGraph, GFXN_GAME, eSetBits);
                        wakeGameTasks();
                    }
                    break;
                    
                case SE_START_SAVED_GAME:
                    // Transición válida: SYS_MAIN_MENU → SYS_IN_GAME  
                    if (currentState == SYS_MAIN_MENU) {
                        currentState = SYS_IN_GAME;
                        // eventos para switch entre menu y juego
                        sendEvent(SE_HIDE_MENU); 			// para UICtrl
						sendEvent(GE_START_SAVED_GAME); 	// para GmeMng
						wakeGameTasks();
                    }
                    break;
                    
                case SE_PAUSE_GAME:
                    // Transición válida: SYS_IN_GAME → SYS_PAUSE
                    if (currentState == SYS_IN_GAME) {
                        currentState = SYS_PAUSE;
                        // TODO: Enviar eventos de pausa
                    }
                    break;
                    
                case SE_RESUME_GAME:
                    // Transición válida: SYS_PAUSE → SYS_IN_GAME
                    if (currentState == SYS_PAUSE) {
                        currentState = SYS_IN_GAME;
                        // TODO: Enviar eventos de reanudación
                    }
                    break;
                    
                case SE_EXIT_GAME:
                    // Transiciones válidas: SYS_IN_GAME → SYS_MAIN_MENU o SYS_PAUSE → SYS_MAIN_MENU
                    if (currentState == SYS_IN_GAME || currentState == SYS_PAUSE) {
                        currentState = SYS_MAIN_MENU;
                        sendEvent(SE_SHOW_MENU);
                    }
                    break;
                    
                default:
                    // Evento no manejado - silencioso
                    break;
            }
        }
    }
}

static void wakeGameTasks(void){
	vTaskResume(tskGameMng);
	vTaskResume(tskRender);
	vTaskResume(tskInpHnd);
}
