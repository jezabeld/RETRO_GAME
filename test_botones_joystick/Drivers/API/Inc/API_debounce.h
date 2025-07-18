/**
 * @file API_debounce.h
 * @brief Implementación de una máquina de estados finitos para el debounce de un botón utilizando retardos no bloqueantes.
 *
 * Creado como parte de la práctica N° 4 de `Programación de Microcontroladores`
 *
 * @author Jez
 * @date Mar 27, 2025
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include <stdint.h>
#include <stdbool.h>
#include <API_delay.h>

/**
 * @typedef debounceState_t
 *
 * @brief Estados de la FSM.
 */
typedef enum{
	BUTTON_UP,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RISING,
} debounceState_t;

/**
 * @typedef buttonChange_t
 *
 * @brief Valores de retorno de la función de actualización de la FSM.
 */
typedef enum{
	BUTTON_UNCHANGED,
	BUTTON_PRESSED,
	BUTTON_RELEASED,
	BUTTON_NOISE_DETECTED,
	BUTTON_ERROR
} buttonChange_t;

/**
 * @brief Inicializa la FSM de debounce.
 *
 * Establece el estado inicial en `BUTTON_UP` y configura el temporizador de debounce.
 */
void debounceFSM_init(void);

/**
 * @brief Actualiza la FSM de debounce.
 *
 * Detecta cambios en el estado del botón aplicando un filtro de debounce basado en retardos.
 *
 * @return variable tipo buttonChange_t con el resultado de la actualización de la FSM.
 */
buttonChange_t debounceFSM_update(void);

/**
 * @brief Lee una variable indicando si el botón fue presionado desde la ultima lectura y luego la reinicia.
 *
 * NOTA: solo contempla que el botón se haya presionado, pero no necesariamente que haya sido soltado.
 *
 * @return `true` si el botón fue presionado desde la última consulta, `false` en caso contrario.
 */
bool_t readKey(void);

#endif /* API_INC_API_DEBOUNCE_H_ */
