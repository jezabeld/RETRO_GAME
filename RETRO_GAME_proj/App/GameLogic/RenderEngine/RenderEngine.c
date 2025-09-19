/*
 * RenderEngine.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "RenderEngine.h"
#include "GameManager.h"
#include "synchronization.h"
#include "ScreenDrv.h"
#include "cmsis_os.h"
#include "systemDefs.h"

extern gameModel_t gameModel;
extern event_id_t gameSignal[10];

renderFrame_t rf = {0};

/* Mapea -100..+100 (pitch) a altura de suelo en píxeles 0..H */
static uint16_t map_pitch_to_ground_px(int16_t pitch100, uint16_t H)
{
    // pitch +100 (trepa) => poco suelo (0 px)
    // pitch -100 (pica)  => mucho suelo (H px)
    int32_t p = pitch100; // -100..+100
    int32_t v = ((100 - p) * (int32_t)H) / 200; // 0..H
    if (v < 0) v = 0; if (v > H) v = H;
    return (uint16_t)v;
}

void RenderEngineTask(void *pvParameters)
{
//	 gameModel_t gameModel;
//	 event_id_t gameSignals[10];

    for(;;)
    {
    	// duerme hasta que GameManager le avise
    	(void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    	/* Frame visual */
    	// renderFrame_t rf = buildFromModel(gameModel, gameSignals);
		rf.scene_cmd   = SCENE_LOAD_GAME;  // la primera vez GraphEngine creará/hará fade; luego podés enviar NONE
		rf.fade_ms     = 250;

		rf.has_ground_h = 1;
		rf.ground_h_px  = map_pitch_to_ground_px(gameModel.pitch, TFT_HEIGHT);
#if DEBUG_LEVEL >= 2
		sendEvent(SE_RDX_RENDER_SENT);
#endif
//        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
