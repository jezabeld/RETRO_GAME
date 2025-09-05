/*
 * EventDispatcher.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "EventDispatcher.h"
#include "synchronization.h"
#include <stdbool.h>
#include "BootMng.h" // aca esta el define de TEST_MODE

// Variable global para control de trace
#ifdef TEST_MODE
#include "FlowAssert.h"
bool traceEnabled = true;
#else
bool traceEnabled = true;
#endif

volatile UBaseType_t uxStackEvntDispTask;
static void routeEvent(event_id_t event);

void EventDispatcherTask(void *pvParameters) {
	event_id_t receivedEvent;
	
	// loop
    for (;;)
    {
    	uxStackEvntDispTask = uxTaskGetStackHighWaterMark(NULL);
        // Esperar eventos en qEvents
        if (xQueueReceive(qEvents, &receivedEvent, portMAX_DELAY) == pdTRUE) {
            // Si trace está activado, reenviar evento a LogSink
            if (traceEnabled) {
                xQueueSend(qLog, &receivedEvent, 0);
            }
#ifdef TEST_MODE
            flowAssertOnEvent(receivedEvent);
#endif
            routeEvent(receivedEvent);

#ifdef TEST_MODE
        flowAssertPoll();   // para que caigan los mensajes de error si un paso no llega a tiempo:
#endif
        }
    }
}

static void routeEvent(event_id_t event) {
    switch (event) {
		case INP_BTN_A:
		case INP_BTN_B:
		case INP_BTN_C:
		case INP_BTN_D:
			// Eventos ya debouncados desde InputDrv → directamente a UI
			xQueueSend(qUiCtrl, &event, 0);
			break;

    	case DBG_TRACE_ON:
    		traceEnabled = true;
    		break;
    	case DBG_TRACE_OFF:
    		traceEnabled = false;
			break;
        // System Events -> qSystem
        case SE_BOOT_COMPLETE:
        case SE_START_NEW_GAME:
        case SE_START_SAVED_GAME:
        case SE_PAUSE_GAME:
        case SE_RESUME_GAME:
        case SE_EXIT_GAME:
            xQueueSend(qSystem, &event, 0);
            break;
            
        // Game Events -> qGame
        case GE_GAME_STARTED:
        case GE_GAME_CONTINUED:
        case GE_GAME_PAUSED:
        case GE_GAME_RESUME:
        case GE_GAME_EXIT:
            xQueueSend(qGame, &event, 0);
            break;
            
        // Config Events
        case CFG_FLAG_REQ:
            xQueueSend(qPersist, &event, 0);
            break;
            
        // Config responses from Persistence -> qUiCtrl
        case CFG_HAS_SAVE:
        case CFG_NO_SAVE:
        case CFG_SAVE_DONE:
        case CFG_SAVE_ERR:
		// UI instructions -> qUiCtrl
        case SE_SHOW_MENU:
        case SE_HIDE_MENU:
            xQueueSend(qUiCtrl, &event, 0);
            break;
            
        // Resource Events -> qResLoad
        case RES_ASSET_READY:
        case RES_ASSET_ERR:
//            xQueueSend(qResLoad, &event, 0);
            break;
            
        // Audio Player Events -> qAudio
        case AUP_BEEP_1:
        case AUP_BEEP_2:
            xQueueSend(qAudio, &event, 0);
            break;
            
        default:
            // Evento desconocido - silencioso
            break;
    }
}


