/**
 * @file API_delay.h
 * @brief Implementación de retardos no bloqueantes.
 *
 * Creado como parte de la práctica N° 3 de `Programación de Microcontroladores`
 *
 * @author Jez
 * @date Mar 20, 2025
 */

#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/**
 * @typedef tick_t
 * @brief Duración de los retardos.
 */
typedef uint32_t tick_t;
/**
 * @typedef bool_t.
 */
typedef bool bool_t;

/**
 * @typedef delay_t
 * @brief Estructura de los retardos.
 */
typedef struct{
   tick_t startTime;
   tick_t duration;
   bool_t running;
} delay_t;

/**
 * @brief Inicializa un retardo no bloqueante.
 *
 * @param[in,out] delay Puntero a la estructura `delay_t` que se va a inicializar.
 * @param[in] duration Duración del retardo en milisegundos (debe ser mayor a 0).
 */
void delayInit( delay_t * delay, tick_t duration );

/**
 * @brief Verifica el estado del retardo y lo inicia si no está corriendo.
 *
 * @param[in,out] delay Puntero a la estructura `delay_t`.
 * @return `true` si el retardo ha finalizado, `false` si aún está en curso.
 */
bool_t delayRead( delay_t * delay );

/**
 * @brief Modifica la duración de un retardo existente.
 *
 * @param[in,out] delay Puntero a la estructura `delay_t`.
 * @param[in] duration Nueva duración del retardo en milisegundos (debe ser mayor a 0).
 */
void delayWrite( delay_t * delay, tick_t duration );

/**
 * @brief Consulta si un retardo está en ejecución.
 *
 * @param[in] delay Puntero a la estructura `delay_t`.
 * @return `true` si el retardo está corriendo, `false` si ha finalizado o no ha iniciado.
 */
bool_t delayIsRunning(delay_t * delay);

#endif /* API_INC_API_DELAY_H_ */
