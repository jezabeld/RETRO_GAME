/*
 * GameManager.h
 *
 *  Mod on: Sept 13, 2025
 *      Author: jez
 */

#ifndef GAMELOGIC_GAMEMANAGER_GAMEMANAGER_H_
#define GAMELOGIC_GAMEMANAGER_GAMEMANAGER_H_

#include "cmsis_os.h"
#include <stdint.h>
#include <stdbool.h>

/* ==== Acciones de gameplay (desde InputHandler a GameManager) ======= */
typedef struct {
    int16_t pitch;     // -100..+100 (normalizado)
    int16_t roll;      // -100..+100
    uint8_t pause;     // 0/1 edge
} gameAction_t;

/* ==== Modelo del juego (GameManager produce) ======================== */
typedef struct {
    int16_t pitch;   // -100..+100  angulo de cabeceo (0: al horizonte)
    int16_t roll;    // -100..+100 angulo de aleteo (0: horizontal)
	int32_t pos_x_map;		// posicion x en el mapa
	int32_t pos_y_map;		// pos y en el mapa
	int16_t heading_deg;	// angulo de direccion o rumbo en el mapa (12 o'clock es el 0)
	int16_t speed_cm_s;		// velocidad de vuelo
	uint32_t game_time_ms;  // “tiempo de vuelo” acumulado
} gameModel_t;

void GameManagerTask(void *pvParameters);

#endif /* GAMELOGIC_GAMEMANAGER_GAMEMANAGER_H_ */
