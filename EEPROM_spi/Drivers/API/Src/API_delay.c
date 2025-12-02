/**
 * @file API_delay.c
 * @brief Implementación de retardos no bloqueantes.
 *
 * @author Jez
 * @date Mar 20, 2025
 */


#include "API_delay.h"

/**
 * @brief Inicializa un delay no bloqueante.
 *
 * @param[in,out] delay Puntero a la estructura `delay_t` que se va a inicializar.
 * @param[in] duration Duración del retardo en milisegundos (debe ser mayor a 0).
 */
void delayInit( delay_t * delay, tick_t duration ){
	assert_param(duration > 0);
	delay->duration = duration;
	delay->running = false;
	return;
}

/**
 * @brief Verifica el estado del delay y lo inicia si no está corriendo.
 *
 * Si el delay no esta corriendo, lo pone a correr.
 * Devuelve TRUE si el delay termino, sino devuelve FALSE.
 *
 * @param[in,out] delay Puntero a la estructura `delay_t`.
 * @return `true` si el retardo ha finalizado, `false` si aún está en curso.
 */

bool_t delayRead( delay_t * delay ){
	assert_param(delay->duration); // checkeo que el delay este inicializado
	if(!delay->running){
		delay->startTime = HAL_GetTick();
		delay->running = true;
	} else {
		if( (HAL_GetTick() - delay->startTime) >= delay->duration ){
			delay->running = false;
			return true;
		}
	}
	return false;
}

/**
 * @brief Modifica la duración de un delay existente.
 *
 * @param[in,out] delay Puntero a la estructura `delay_t`.
 * @param[in] duration Nueva duración del delay en milisegundos (debe ser mayor a 0).
 */
void delayWrite( delay_t * delay, tick_t duration ){
	assert_param(duration > 0);
	delay->duration = duration;
}

/**
 * @brief Consulta si un delay está en ejecución.
 *
 * @param[in] delay Puntero a la estructura `delay_t`.
 * @return `true` si el delay está corriendo, `false` si ha finalizado o no ha iniciado.
 */
bool_t delayIsRunning(delay_t * delay){
	assert_param(delay->duration);
	return delay->running;
}
