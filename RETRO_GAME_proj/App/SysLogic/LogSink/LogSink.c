/*
 * LogSink.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */


#include "LogSink.h"
#include "CommandParser.h"
#include "UARTDrv.h"
#include "events.h"
#include "cmsis_os.h"

/* ======================
 * Tarea LogSink
 * ====================== */
void LogSinkTask(void *pvParameters)
{
    event_id_t receivedEvent;

    for (;;)
    {
        // Esperar eventos de log/trace en qLog
        if (xQueueReceive(qLog, &receivedEvent, portMAX_DELAY) == pdTRUE)
        {
            // Recibir eventos reenviados por EventDispatcher para trace
            uartSendString("TRACE: evento ");
            uartSendValue(receivedEvent);
            uartSendString(" detectado\r\n");
        }
        
    }
}
