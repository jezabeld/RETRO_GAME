/*
 * GraphEngine.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "GraphEngine.h"
#include "synchronization.h"
#include "systemDefs.h"
#include "cmsis_os.h"
#include "ScreenDrv.h"
#include "lvgl.h"
#include "lv_port_disp.h" /* <- your display driver registration function   */
#include "lv_port_indev.h" /* <- your input device driver registration function */
#include "gameScreen.h"
#include "RenderEngine.h"

typedef enum { GFX_UI = 0, GFX_GAME } gfx_mode_t;
typedef enum { GFX_CMD_ENTER_UI = 1, GFX_CMD_ENTER_GAME } gfx_cmd_t;

extern tft_t myTft;
volatile UBaseType_t uxStackGFXTask;
static gfx_mode_t gfxMode = GFX_UI;

extern renderFrame_t rf;

static void updateGFXmode(void){
	uint32_t flags = 0;
	(void)xTaskNotifyWait(
		0,                  /* no limpia al entrar */
		0xFFFFFFFFu,        /* limpia todos al salir */
		&flags,
		0                   /* timeout 0 -> no bloqueante */
	);
	if (flags & GFXN_GAME) {
		game_screen_create();
		game_screen_show(true);
		gfxMode = GFX_GAME;
		sendEvent(SE_GFX_GAME_MODE);
	}
	if (flags & GFXN_UI) {
		gfxMode = GFX_UI;
	}

}
static void apply_frame(const renderFrame_t *rf) {
    if (rf->scene_cmd == SCENE_LOAD_GAME) {
        game_screen_create();
        game_screen_show(rf->fade_ms > 0);
        gfxMode = GFX_GAME;
    }
    if (rf->has_ground_h) game_screen_set_ground_height_px(rf->ground_h_px);
}

void GraphEngineTask(void *pvParameters)
{
    // Fase 1: Inicializar LVGL
	lv_init();
    lv_port_disp_init(); /* <-- registra display */
    lv_port_indev_init(); /* <-- registra input devices */

    // Señalizar que GraphEngine está listo 
    xSemaphoreGive(semGFXReady);
    
#if DEBUG_LEVEL >= 2
    // Enviar evento para FlowPlans indicando que LVGL está inicializado
    event_id_t gfxInitEvent = SE_GFX_INIT;
    xQueueSend(qEvents, &gfxInitEvent, 0);
#endif
    
    // Esperar que UI esté lista
    uxStackGFXTask = uxTaskGetStackHighWaterMark(NULL);
    xSemaphoreTake(semUiReady, portMAX_DELAY);
    
    // Fase 2: Loop principal LVGL con timing exacto
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(GFX_TICK_MS); // 20 FPS exactos
    
    for(;;)
    {
#if DEBUG_LEVEL >= 2
        // Enviar evento SE_GFX_RUNNING para indicar actividad (solo para test)
        event_id_t gfxEvent = SE_GFX_RUNNING;
        xQueueSend(qEvents, &gfxEvent, 0);
#endif
        // leer notificaciones para actualizar modo grafico
        updateGFXmode();

        
        lv_timer_handler(); /* procesa LVGL */
		lv_port_indev_clear_buttons(); /* clear button states after processing */

		/* en GAME aplicar frames pendientes */
		if (gfxMode == GFX_GAME) {
//			while (xQueueReceive(qRenderFrame, &rf, 0) == pdPASS) {
				apply_frame(&rf);
//			}
		}

		uxStackGFXTask = uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&xLastWakeTime, xFrequency); // Timing exacto cada 50ms
    }
}
