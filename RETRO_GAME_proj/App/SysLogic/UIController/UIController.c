/*
 * UIController.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "UIController.h"
#include "events.h"
#include "systemDefs.h"
#include "cmsis_os.h"

void UIControllerTask(void *pvParameters)
{
    event_id_t receivedEvent;
    
    for(;;)
    {
        // Esperar eventos en qUiCtrl con timeout para no bloquear indefinidamente
        if (xQueueReceive(qUiCtrl, &receivedEvent, pdMS_TO_TICKS(2000)) == pdTRUE) {
            
            switch (receivedEvent) {
                case CFG_HAS_SAVE:
                    // Habilitar opción "Continuar" en menú
                    break;
                    
                case CFG_NO_SAVE:
                    // Deshabilitar opción "Continuar" en menú
                    break;
                    
                default:
                    // Evento no manejado - silencioso
                    break;
            }
        } else {
            // Timeout - UI sigue funcionando
        }
    }
}