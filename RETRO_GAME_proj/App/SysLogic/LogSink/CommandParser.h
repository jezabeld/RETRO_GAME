/*
 * CommandParser.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef SYSLOGIC_LOGSINK_COMMANDPARSER_H_
#define SYSLOGIC_LOGSINK_COMMANDPARSER_H_

#include <stdint.h>
#include <stdbool.h>

/* Configuración */
#ifndef CMD_LINE_MAX
#define CMD_LINE_MAX     64    // Tamaño máx. de un comando
#endif

void UARTDrv_OnRxData(const uint8_t *data, uint16_t len);

#endif /* SYSLOGIC_LOGSINK_COMMANDPARSER_H_ */
