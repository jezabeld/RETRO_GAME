/*
 * UARTDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_UARTDRV_H_
#define DRIVERS_UARTDRV_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

// API con ring buffer DMA
int uartInit(void);
int uartSend(const void *buf, uint16_t len);       // Función base - encola datos binarios
int uartSendString(const char *pstring);           // Encola string (calcula longitud)
int uartSendStringSize(const char *pstring, uint16_t size); // Encola string con longitud conocida
int uartSendValue(int32_t value);                  // Encola valor como string

// Hook débil para RX (redefinible)
void UARTDrv_OnRxData(const uint8_t *data, uint16_t len);

#endif /* DRIVERS_UARTDRV_H_ */
