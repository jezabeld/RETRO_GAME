/*
 * CommandParser.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "CommandParser.h"
#include "UARTDrv.h"
#include "synchronization.h"
#include <string.h>

static char     s_line[CMD_LINE_MAX];
static uint16_t s_len = 0;

static void parserHandleLine(const char* line);

void UARTDrv_OnRxData(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        char c = (char)data[i];

        // Normalizar \r\n: convierte \r en \n y colapsa dobles finales
        if (c == '\r') c = '\n';

        if (c == '\n')
        {
            // Terminar línea y parsear si hay algo
            if (s_len > 0) {
                s_line[s_len] = '\0';
                parserHandleLine(s_line);
                s_len = 0;
            }
        }
        else
        {
            if (s_len < (CMD_LINE_MAX - 1)) {
                s_line[s_len++] = c;
            } else {
                // Overflow: descarta la línea actual
                s_len = 0;
            }
        }
    }
}

static void parserHandleLine(const char* line)
{
	// Opcional: trim simple
	while (*line == ' ') line++;

	event_id_t ev = 0xFF;

	if (strcasecmp(line, "logon")  == 0) ev = DBG_TRACE_ON;
	if (strcasecmp(line, "logoff") == 0) ev = DBG_TRACE_OFF;

	if (ev != 0xFF) {
		BaseType_t xHPW = pdFALSE;
		(void)xQueueSendFromISR(qEvents, &ev, &xHPW);
		portYIELD_FROM_ISR(xHPW);
	}

}

