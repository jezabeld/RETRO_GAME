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

UART_HandleTypeDef huart2;
// la definición de la variable siempre va en un archivo fuente ya que los headers se pueden importar múltiples veces
// la declaración con `extern` en el header hace que la variable sea visible desde main.c

static void intToStr(int32_t num, char* str);

HAL_StatusTypeDef uartInit(){
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK){
		return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef uartSendString(const char *pstring){
	assert(pstring);
	assert_param(strlen(pstring) <= (2^sizeof(uint16_t)));

	HAL_StatusTypeDef status;
	uint8_t retries= N_RETRIES;

	do{
		status = HAL_UART_Transmit(&huart2, (uint8_t *)pstring, (uint16_t)strlen(pstring), HAL_MAX_DELAY);
	}while( status != HAL_OK && (retries--));

	return status;
}

HAL_StatusTypeDef uartSendStringSize(const char * pstring, uint16_t size){
	assert(pstring);
	assert_param(strlen(pstring) <= (2^sizeof(uint16_t)));
	assert_param(size > 0);
	assert_param(size <= strlen(pstring));

	HAL_StatusTypeDef status;
	uint8_t retries= N_RETRIES;

	do{
		status = HAL_UART_Transmit(&huart2, (uint8_t *)pstring, size, HAL_MAX_DELAY);
	}while(status != HAL_OK && (retries--));

	return status;
}

HAL_StatusTypeDef uartReceiveStringSize(char * pstring, uint16_t size){
	assert(pstring);
	assert_param(strlen(pstring) <= (2^sizeof(uint16_t)));
	assert_param(size > 0);
	assert_param(size <= strlen(pstring));

	HAL_StatusTypeDef status;
	uint8_t retries= N_RETRIES;

	do{
		status = HAL_UART_Receive(&huart2, (uint8_t *)pstring, size, HAL_MAX_DELAY);
	}while(status != HAL_OK && (retries--));

	return status;
}

HAL_StatusTypeDef uartSendValue(int32_t value){
	char buffer[12]; // max 10 digitos mas signo mas null character = 12
	intToStr(value, buffer);

	HAL_StatusTypeDef status = uartSendString(buffer);

	return status;
}

void intToStr(int32_t num, char* str){
	uint8_t i = 0;
	uint8_t neg = 0;

	if (num == 0) {
		str[i++] = '0';
	}
	if(num < 0){
		//str[i++] = '-';
		neg = 1;
		num = num * (-1);
	}
	while (num > 0) {
		str[i++] = (num % 10) + '0';
		num /= 10;
	}
	if(neg){
			str[i++] = '-';
		}
	str[i] = '\0';

	// Invertir el string
	for (uint8_t j = 0; j < i / 2; j++) {
		char temp = str[j];
		str[j] = str[i - j - 1];
		str[i - j - 1] = temp;
	}

}
