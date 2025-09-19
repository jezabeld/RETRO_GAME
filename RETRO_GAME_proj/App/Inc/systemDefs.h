/*
 * systemDefs.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef INC_SYSTEMDEFS_H_
#define INC_SYSTEMDEFS_H_

#include <stdint.h>

#define GFX_TICK_MS 50

// Niveles de debug para desarrollo
// 0: NO DEBUG
// 1: DEBUG LIGERO - habilita timers y traceEnabled
// 2: FULL DEBUG - todo lo anterior + funcionalidades adicionales
#define DEBUG_LEVEL 1

/* ==============================
 * Estados globales del sistema
 * ============================== */

typedef struct {
	int32_t pos_x_map;		// posicion x en el mapa
	int32_t pos_y_map;		// pos y en el mapa
	int16_t heading_deg;	// angulo de direccion o rumbo en el mapa (12 o'clock es el 0)
	int16_t speed_cm_s;		// velocidad de vuelo
	uint32_t game_time_ms;  // “tiempo de vuelo” acumulado
	uint8_t  crc;           // CRC-8 del bloque
} SaveState_t;

typedef enum {
	PAU_MENU = 0,
	PAU_SAVING,
	PAU_SAVE_OK,
	PAU_SAVE_ERR
} syspause_state_t;

#endif /* INC_SYSTEMDEFS_H_ */
