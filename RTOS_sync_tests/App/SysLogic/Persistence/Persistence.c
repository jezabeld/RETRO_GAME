/*
 * Persistence.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "Persistence.h"
#include "events.h"
#include "systemDefs.h"
#include "cmsis_os.h"

void PersistenceTask(void *pvParameters)
{
    event_id_t receivedEvent;
    
    for(;;)
    {
        // Esperar eventos en qPersist
        if (xQueueReceive(qPersist, &receivedEvent, portMAX_DELAY) == pdTRUE) {
            
            switch (receivedEvent) {
                case CFG_FLAG_REQ:
                    // Simular verificación de EEPROM
                    vTaskDelay(pdMS_TO_TICKS(100));
                    
                    // Simular que no hay partida guardada (según flujo del README)
                    event_id_t responseEvent = CFG_NO_SAVE;
                    xQueueSend(qEvents, &responseEvent, 0);
                    break;
                    
                case CFG_SAVE_DONE:
                    // Guardado completado
                    break;
                    
                case CFG_SAVE_ERR:
                    // Error en guardado
                    break;
                    
                default:
                    // Evento no manejado - silencioso
                    break;
            }
        }
    }
}
