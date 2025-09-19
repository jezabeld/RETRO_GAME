/*
 * gameScreen.h
 *
 *  Created on: Sep 11, 2025
 *      Author: jez
 */

#ifndef GAMELOGIC_RENDERENGINE_GAMESCREEN_H_
#define GAMELOGIC_RENDERENGINE_GAMESCREEN_H_

#include "lvgl.h"

/* API pública de la “screen” del juego */
void game_screen_create(void);                   /* Construye objetos (una vez) */
void game_screen_show(bool fade_in);             /* La hace activa (con o sin fade) */
void game_screen_destroy(void);                  /* Libera todo (opcional) */

/* Actualizaciones desde tu loop/timer */
void game_screen_set_ground_height_px(uint16_t h_px);     /* 0..ver_res */
void game_screen_set_pitch_raw(uint16_t adc0_4095);       /* helper de mapeo 0..4095 -> 0..ver_res */

#endif /* GAMELOGIC_RENDERENGINE_GAMESCREEN_H_ */
