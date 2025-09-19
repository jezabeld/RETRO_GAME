/*
 * GameManager.c
 *
 *  Mod on: Sept 13, 2025
 *      Author: jez
 */

#include "GameManager.h"
#include "systemDefs.h"
#include "synchronization.h"

/* ======= Config ======= */
#define GM_TICK_MS      20      // 50 Hz

/* Estados de la FSM de juego */
typedef enum {
    GME_STOPPED = 0,    // Juego no iniciado o terminado
	GME_LOADING,
	GME_ERRLOAD,
    GME_RUNNING,        // Juego en ejecución
    GME_FROZEN          // Juego pausado
} gameState_t;

/* ======= Estado interno ======= */
extern TaskHandle_t tskGameMng;
extern TaskHandle_t tskRender;
static volatile gameState_t sState = GME_STOPPED;
const TickType_t xFrequency = pdMS_TO_TICKS(GFX_TICK_MS); // 20 FPS exactos
gameModel_t gameModel = {0}; // TODO: inicializar en estado conocido
event_id_t gameSignal[10]; // si defino las seniales directamente como los eventos que las triguerean es mas facil de traducir
uint8_t sigCount = 0;   // cuántas señales hay cargadas (0..10)

void GameManagerTask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
    	/* Procesar TODOS los eventos pendientes*/
		event_id_t ev;
		while (xQueueReceive(qGame, &ev, 0) == pdPASS) {
			// Máquina de estados del juego
			switch (ev) {
				case GE_START_NEW_GAME:
					// Transición válida: GME_STOPPED → GME_RUNNING
					if (sState == GME_STOPPED) {
						sState = GME_RUNNING;
						// TODO: Inicializar nuevo juego
						sendEvent(GE_GAME_STARTED);
					}
					break;
					
				case GE_START_SAVED_GAME:
					// Transición válida: GME_STOPPED → GME_LOADING
					if (sState == GME_STOPPED) {
						sState = GME_LOADING;
						// TODO: Iniciar proceso de carga
					}
					break;
					
				case GE_LOAD_OK:
					// Transición válida: GME_LOADING → GME_RUNNING
					if (sState == GME_LOADING) {
						sState = GME_RUNNING;
						sendEvent(GE_GAME_CONTINUED);
					}
					break;
					
				case GE_LOAD_ERR:
					// Transición válida: GME_LOADING → GME_ERRLOAD
					if (sState == GME_LOADING) {
						sState = GME_ERRLOAD;
						// TODO: Manejar error de carga
						// Implementar timeout para ir a GME_RUNNING
					}
					break;
					
				case GE_GAME_PAUSED:
					// Transición válida: GME_RUNNING → GME_FROZEN
					if (sState == GME_RUNNING) {
						sState = GME_FROZEN;
						// TODO: Pausar lógica de juego
					}
					break;
					
				case GE_GAME_RESUME:
					// Transición válida: GME_FROZEN → GME_RUNNING
					if (sState == GME_FROZEN) {
						sState = GME_RUNNING;
						// TODO: Reanudar lógica de juego
					}
					break;
					
				case GE_GAME_EXIT:
					// Transiciones válidas: GME_RUNNING/GME_FROZEN → GME_STOPPED
					if (sState == GME_RUNNING || sState == GME_FROZEN) {
						sState = GME_STOPPED;
						// TODO: Limpiar estado del juego
					}
					break;
					
				default:
					// Evento no manejado - silencioso
					break;
			}
		}



        // si el juego está corriendo tomar la ÚLTIMA acción disponible y actualizar el modelo
        if (sState == GME_RUNNING) {

        	gameAction_t act = {0}, tmp;

            /* Se queda con la última acción encolada (si hay varias) */
			while (xQueueReceive(qActions, &tmp, 0) == pdPASS) act = tmp;

            /* GAME UPDATE: Ejemplo mínimo: copiar inputs al modelo */
			// gameUpdate(&gameModel);
			gameModel.pitch = act.pitch;
			gameModel.roll  = act.roll;

            /* Publicar modelo + señales para Render/Audio/Haptic */
			xTaskNotifyGive(tskRender);
//			(void)xQueueOverwrite(qGameModel, &sModel);
//			if(sigCount){
//				uint8_t i = 0;
//				while (i < sigCount) {
//					sendEventQueue(gameSignal[i++], qRender);
//				}
//				sigCount = 0;  // vaciar
//			}
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency); // Timing exacto cada 50ms
    }
}

