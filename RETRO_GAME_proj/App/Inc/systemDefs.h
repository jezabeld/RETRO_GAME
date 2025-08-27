/*
 * systemDefs.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef INC_SYSTEMDEFS_H_
#define INC_SYSTEMDEFS_H_

#include <stdint.h>

/* ==============================
 * Estados globales del sistema
 * ============================== */

/* Estados de la FSM principal del sistema */
typedef enum {
    SYS_BOOT = 0,       // Arranque y autodiagnóstico
    SYS_SPLASH,         // Pantalla inicial
    SYS_MAIN_MENU,      // Menú principal
    SYS_IN_GAME,        // Ejecutando gameplay
    SYS_PAUSE      // Menú de pausa
} sys_state_t;

/* Estados de la FSM de juego */
typedef enum {
    GME_STOPPED = 0,    // Juego no iniciado o terminado
	GME_LOADING,
	GME_ERRLOAD,
    GME_RUNNING,        // Juego en ejecución
    GME_FROZEN          // Juego pausado
} game_state_t;

typedef enum {
	PAU_MENU = 0,
	PAU_SAVING,
	PAU_SAVE_OK,
	PAU_SAVE_ERR
} syspause_state_t;

///* Estados de la FSM de recursos */
//typedef enum {
//    RES_IDLE = 0,       // No cargando
//    RES_LOADING,        // Cargando recursos
//    RES_READY,          // Recursos cargados
//    RES_ERROR           // Fallo en carga
//} res_state_t;
//
///* Estados de persistencia */
//typedef enum {
//    PRS_IDLE = 0,       // No hay operación
//    PRS_SAVING,         // Guardando datos
//    PRS_LOADING,        // Cargando datos
//    PRS_DONE,           // Operación exitosa
//    PRS_ERROR           // Error en operación
//} persist_state_t;
//
///* Estados del subsistema de audio */
//typedef enum {
//    AUD_IDLE = 0,
//    AUD_PLAYING,
//    AUD_ERROR
//} audio_state_t;
//
///* Estados del subsistema háptico */
//typedef enum {
//    HAP_IDLE = 0,
//    HAP_PLAYING,
//    HAP_ERROR
//} haptic_state_t;


#endif /* INC_SYSTEMDEFS_H_ */
