/*
 * synchronization.h
 *
 * Definiciones de eventos, colas y semáforos de sincronización del sistema
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef INC_SYNCHRONIZATION_H_
#define INC_SYNCHRONIZATION_H_

#include <stdint.h>
#include "cmsis_os.h"

/* ======================
 * Declaraciones externas de colas del sistema
 * ====================== */
extern QueueHandle_t qEvents;
extern QueueHandle_t qResLoad;
extern QueueHandle_t qPersist;
extern QueueHandle_t qUiCtrl;
extern QueueHandle_t qGraph;
extern QueueHandle_t qAudio;
extern QueueHandle_t qHaptic;
extern QueueHandle_t qLog;
extern QueueHandle_t qSystem;
extern QueueHandle_t qGame;
extern QueueHandle_t qInHand;

/* ======================
 * Semáforos de sincronización entre tareas
 * ====================== */
extern SemaphoreHandle_t semGraphEngineReady;  // GraphEngine → UIController  
extern SemaphoreHandle_t semUiReady;           // UIController → GraphEngine

/* ======================
 * Eventos del sistema
 * ====================== */

// tipo de dato para los eventos
typedef uint8_t event_id_t; // tipo real para colas, estructuras, etc.

/* Se usa un enum solo como namespace de constantes
 * ya que no garantiza que sea uint8_t — siempre es un tipo entero con al menos 16 bits.
 */
enum {
/* ======================
 * System Events (SE_)
 * ====================== */
    SE_BOOT_COMPLETE = 0, // BootMng: drivers & RTOS OK → habilita main-menu
    SE_START_NEW_GAME,    // UI: SYS_MAIN_MENU → SYS_IN_GAME (nuevo)
    SE_START_SAVED_GAME,  // UI: SYS_MAIN_MENU → SYS_IN_GAME (cargar)
    SE_PAUSE_GAME,        // InputHandler: pausa
    SE_RESUME_GAME,       // UI: reanuda
    SE_EXIT_GAME,          // UI: salir al menú principal
	SE_SHOW_MENU,
	SE_HIDE_MENU,
	SE_GFX_RUNNING,        // GraphEngine: indicador de actividad

/* ======================
 * Game Events (GE_)
 * ====================== */
    GE_GAME_STARTED = 10,
    GE_GAME_CONTINUED,
    GE_GAME_PAUSED,
    GE_GAME_RESUME,
    GE_GAME_EXIT,

/* ======================
 * Config / Persist (CFG_)
 * ====================== */
	CFG_FLAG_REQ = 20,  // BootMng: solicita chequeo de partida guardada
    CFG_HAS_SAVE,      // ConfigPersist: habilitar “Continue”
    CFG_NO_SAVE,       // ConfigPersist: deshabilitar “Continue”
    CFG_SAVE_DONE,     // Persistencia exitosa
    CFG_SAVE_ERR,       // Persistencia fallida

/* ======================
 * Resource Loader (RES_)
 * ====================== */
    RES_ASSET_READY = 30,
    RES_ASSET_ERR,

/* ======================
 * Input crudos (RAW_)
 * ====================== */
	RAW_BTN_A = 50,
	RAW_BTN_B,
	RAW_BTN_C,
	RAW_BTN_D,

/* ======================
 * Input procesados (INP_)
 * ====================== */
	INP_BTN_A = 55,
	INP_BTN_B,
	INP_BTN_C,
	INP_BTN_D,

/* ======================
 * Drivers: TFT_, AUD_, HAP_, DBG_, INP_, MEM_, TIM_
 * ====================== */
    TFT_OK = 80,
    TFT_ERR,
    AUD_OK,
    AUD_ERR,
    HAP_OK,
    HAP_ERR,
    DBG_OK,
    DBG_ERR,
    INP_OK,
    INP_ERR,
    MEM_OK,
    MEM_ERR,
    TIM_OK,
    TIM_ERR,

/* ======================
 * Debug (DBG_)
 * ====================== */
    DBG_TRACE_ON = 100,
    DBG_TRACE_OFF,
};



#endif /* INC_SYNCHRONIZATION_H_ */
