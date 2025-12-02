/**
 * @file API_uart.c
 * @brief Módulo para la utilización de la UART2.
 *
 * Creado como parte de la práctica N° 5 de `Programación de Microcontroladores`
 *
 * @author Jez
 * @date Apr 4, 2025
 */

#include "API_uart.h"
#include "string.h"
#include "stm32f4xx_hal.h"

#define N_RETRIES 3


UART_HandleTypeDef huart2;
// la definición de la variable siempre va en un archivo fuente ya que los headers se pueden importar múltiples veces
// la declaración con `extern` en el header hace que la variable sea visible desde main.c

bool_t uartInit(){
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK){
		return false;
	}
	return true;
}

void uartSendString(char *pstring){
	/* La practica indica que la funcion debe recibir un entero de 8 bits
	 * pero me pareció mejor que tome un string y lo castee ya que lo hace mas user friendly.
	 */
	assert(pstring);
	assert_param(strlen(pstring) <= (2^sizeof(uint16_t)));

	HAL_StatusTypeDef status;
	uint8_t retries= N_RETRIES;

	do{
		status = HAL_UART_Transmit(&huart2, (uint8_t *)pstring, (uint16_t)strlen(pstring), HAL_MAX_DELAY);
	}while( status != HAL_OK && (retries--));
	// podria retornar el status final
}

void uartSendStringSize(char * pstring, uint16_t size){
	/* La practica indica que la funcion debe recibir un entero de 8 bits
	 * pero me pareció mejor que tome un string y lo castee ya que lo hace mas user friendly.
	 */
	assert(pstring);
	assert_param(strlen(pstring) <= (2^sizeof(uint16_t)));
	assert_param(size > 0);
	assert_param(size <= strlen(pstring));

	HAL_StatusTypeDef status;
	uint8_t retries= N_RETRIES;

	do{
		status = HAL_UART_Transmit(&huart2, (uint8_t *)pstring, size, HAL_MAX_DELAY);
	}while(status != HAL_OK && (retries--));
	// podria retornar el status final
}

void uartReceiveStringSize(char * pstring, uint16_t size){
	/* La practica indica que la funcion debe recibir un entero de 8 bits
	 * pero me pareció mejor que tome un string y lo castee ya que lo hace mas user friendly.
	 */
	assert(pstring);
	assert_param(strlen(pstring) <= (2^sizeof(uint16_t)));
	assert_param(size > 0);
	assert_param(size <= strlen(pstring));

	HAL_StatusTypeDef status;
	uint8_t retries= N_RETRIES;

	do{
		status = HAL_UART_Receive(&huart2, (uint8_t *)pstring, size, HAL_MAX_DELAY);
	}while(status != HAL_OK && (retries--));
	// podria retornar el status final
}

void uartSendValue(int16_t * value){
	char buffer[6]; // 5 dígitos + null terminator
	snprintf(buffer, sizeof(buffer), "%d", *value); // convierte el número a string
	uartSendString(buffer);
}
