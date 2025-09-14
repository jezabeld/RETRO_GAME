/*
 * UIController.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "UIController.h"
#include "synchronization.h"
#include "systemDefs.h"
#include "cmsis_os.h"
#include "ui.h"
#include "lv_port_indev.h"
#include "lv_port_GFX.h"

extern volatile uint16_t joy_raw[2];
extern TaskHandle_t tskGraph;
extern lv_obj_t *ui_SYSMAINMENU;

volatile UBaseType_t uxStackUiTask;

typedef enum { SAVE_UNKNOWN=0, SAVE_NONE, SAVE_PRESENT } save_state_t;

static bool         want_show_menu = false;
static bool         gfx_ready      = false;
static save_state_t save_state     = SAVE_UNKNOWN;
static bool         ui_inited      = false;

// Función helper para inicializar UI si están listas ambas condiciones
static void try_initialize_ui(void) {
    if (!ui_inited && want_show_menu && gfx_ready && (save_state != SAVE_UNKNOWN)) {

        // Inicializar el menú según el estado del save
        ui_init(save_state == SAVE_PRESENT);
        
        // Señalizar que UI está lista
        ui_inited = true;
        xSemaphoreGive(semUiReady);
    }
}

void UIControllerTask(void *pvParameters)
{
    event_id_t receivedEvent;
    
    for(;;)
    {
    	uxStackUiTask = uxTaskGetStackHighWaterMark(NULL);
        if (xQueueReceive(qUiCtrl, &receivedEvent, portMAX_DELAY)) {
            
            switch (receivedEvent) {
                case CFG_HAS_SAVE:
                	save_state = SAVE_PRESENT;
                    try_initialize_ui();
                    break;
                    
                case CFG_NO_SAVE:
                	save_state = SAVE_NONE;
                    try_initialize_ui();
                    break;
                    
                case SE_SHOW_MENU:
                	want_show_menu = true;
                    try_initialize_ui();
                    break;
				case SE_HIDE_MENU:
					// Debug: verificar que se ejecute
					sendEvent(SE_UI_HIDDEN);
					
					// Ejecutar fadeout
					if (ui_SYSMAINMENU) {
						ui_fadeout_screen(ui_SYSMAINMENU, 500); // fade de 500 ms
						sendEvent(SE_UI_SUSPENDING);
						vTaskSuspend(NULL); // suspenderse hasta que se necesiten los menus
					}
					break;

                // Botones
				case INP_BTN_A:
					lv_port_indev_btn_a_pressed();
					// Enviar evento de sonido para botón A
					sendEvent(AUP_BEEP_1);
					break;
				case INP_BTN_B:
					lv_port_indev_btn_b_pressed();
					// Enviar evento de sonido para botón B
					sendEvent(AUP_BEEP_2);
					break;
				case INP_BTN_C:
					lv_port_indev_btn_c_pressed();
					// Enviar evento de sonido para botón C
					sendEvent(AUP_BEEP_3);
					break;
				case INP_BTN_D:
					lv_port_indev_btn_d_pressed();
					// Enviar evento de sonido para botón D
					sendEvent(AUP_BEEP_4);
					break;

                default:
                    // Evento no manejado - silencioso
                    break;
            }
            if (!gfx_ready) {
				// Reviso el semaforo de GFX de forma no bloqueante
				if (xSemaphoreTake(semGFXReady, 0) == pdTRUE) {
					gfx_ready = true;
				}
			}
        } 
    }
}
