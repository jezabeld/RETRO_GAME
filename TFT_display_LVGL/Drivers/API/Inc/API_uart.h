/**
 * @file API_uart.h
 * @brief Módulo para la utilización de la UART2.
 *
 * Creado como parte de la práctica N° 5 de `Programación de Microcontroladores`
 *
 * @author Jez
 * @date Apr 4, 2025
 */

#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_

#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

extern UART_HandleTypeDef huart2;

#define N_RETRIES 3

/**
 * @brief Inicializa la UART2.
 *
 * @return `HAL_OK` si la inicialización se ha realizado con exito, en otro caso retorna `HAL_ERROR`.
 */
HAL_StatusTypeDef uartInit();

/**
 * @brief Transmite un string por la UART.
 *
 * Si la transmisión falla, reintentará un total de veces definido en la macro N_RETRIES.
 *
 * @param pstring puntero a un string completo (incluyendo el caracter ‘\0’).
 *  pstring no puede ser mayor que 2^16 caracteres.
 *
 * @return status final despues de los reintentos.
 */
HAL_StatusTypeDef uartSendString(char * pstring);

/**
 * @brief Transmite un string por la UART hasta un determinado número de caracteres.
 *
 * Si la transmisión falla, reintentará un total de veces definido en la macro N_RETRIES.
 *
 * @param pstring puntero a un string completo (incluyendo el caracter ‘\0’).
 *  pstring no puede ser mayor que 2^16 caracteres.
 *
 * @param size cantidad de caracteres de pstring a enviar.
 * 	size debe ser mayor a cero y no debe ser mayor que la cantidad de caracteres (incluyendo el caracter NULL) de pstring.
 *
 * @return status final despues de los reintentos.
 */
HAL_StatusTypeDef uartSendStringSize(char * pstring, uint16_t size);

/**
 * @brief Recive un string por la UART.
 *
 * Si la recepción falla, reintentará un total de veces definido en la macro N_RETRIES.
 *
 * @param pstring puntero a un string donde se guardará el mensaje obtenido.
 *  pstring no puede ser mayor que 2^16 caracteres.
 *
 * @param size cantidad de caracteres a recibir.
 * 	size debe ser mayor a cero y no debe ser mayor que la cantidad de caracteres (incluyendo el caracter NULL) de pstring.
 *
 * @return status final despues de los reintentos.
 */
HAL_StatusTypeDef uartReceiveStringSize(char * pstring, uint16_t size);

/**
 * @brief Transmite un número por la UART.
 *
 * Si la transmisión falla, reintentará un total de veces definido en la macro N_RETRIES.
 *
 * @param value número entero de hasta 32 bits.
 *
 * @return status final despues de los reintentos.
 */
HAL_StatusTypeDef uartSendValue(int32_t value);

#endif /* API_INC_API_DELAY_H_ */
