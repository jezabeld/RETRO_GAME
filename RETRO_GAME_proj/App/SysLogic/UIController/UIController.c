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

extern volatile uint16_t joy_raw[2];
extern TaskHandle_t tskGraph;

volatile UBaseType_t uxStackUiTask;

void UIControllerTask(void *pvParameters)
{
    event_id_t receivedEvent;
    
    for(;;)
    {
    	uxStackUiTask = uxTaskGetStackHighWaterMark(NULL);
        if (xQueueReceive(qUiCtrl, &receivedEvent, portMAX_DELAY)) {
            
            switch (receivedEvent) {
                case CFG_HAS_SAVE:
                    // Habilitar opción "Continuar" en menú
                    break;
                    
                case CFG_NO_SAVE:
                    // Deshabilitar opción "Continuar" en menú
                    break;
                    
                case SE_SHOW_MENU:
                	// Esperar que LVGL esté inicializado
                	xSemaphoreTake(semGFXReady, portMAX_DELAY);
                	
                	// Inicializar el menú
                	ui_init();
                	
                	// Señalizar que UI está lista
                	xSemaphoreGive(semUiReady);
                	break;

                // Botones
				case INP_BTN_A:
					lv_port_indev_btn_a_pressed();
					break;
				case INP_BTN_B:
					lv_port_indev_btn_b_pressed();
					break;
				case INP_BTN_C:
					lv_port_indev_btn_c_pressed();
					break;
				case INP_BTN_D:
					lv_port_indev_btn_d_pressed();
					break;

                default:
                    // Evento no manejado - silencioso
                    break;
            }
        } 
    }
}
