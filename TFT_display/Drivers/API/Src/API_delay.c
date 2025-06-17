/**
 * @file API_delay.c
 * @brief Implementación de retardos no bloqueantes.
 *
 * Creado como parte de la práctica N° 3 de `Programación de Microcontroladores`
 *
 * @author Jez
 * @date Mar 20, 2025
 */

#include "API_delay.h"

void delayInit( delay_t * delay, tick_t duration ){
	assert_param(duration > 0);
	delay->duration = duration;
	delay->running = false;
	return;
}

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

void delayWrite( delay_t * delay, tick_t duration ){
	assert_param(duration > 0);
	delay->duration = duration;
}

bool_t delayIsRunning(delay_t * delay){
	assert_param(delay->duration);
	return delay->running;
}
