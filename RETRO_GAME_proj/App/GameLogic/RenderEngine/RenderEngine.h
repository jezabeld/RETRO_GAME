/*
 * RenderEngine.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef GAMELOGIC_RENDERENGINE_RENDERENGINE_H_
#define GAMELOGIC_RENDERENGINE_RENDERENGINE_H_

#include "cmsis_os.h"

/* ==== Comandos de escena y frame de render ========================== */
typedef enum {
    SCENE_NONE = 0,
    SCENE_LOAD_GAME,
} sceneCmd_e;

typedef struct {
    sceneCmd_e scene_cmd;
    uint16_t   fade_ms;           // para LOAD/TRANSICIONES

    /* “OPS” mínimos de este prototipo */
    uint8_t    has_ground_h;      // set=1 si aplica
    uint16_t   ground_h_px;       // 0..ver_res

    /* Extensible: sky_color, HUD, sprites, parallax, etc. */
} renderFrame_t;

void RenderEngineTask(void *pvParameters);

#endif /* GAMELOGIC_RENDERENGINE_RENDERENGINE_H_ */
