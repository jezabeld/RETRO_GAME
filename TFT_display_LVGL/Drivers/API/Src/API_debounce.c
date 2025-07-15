/**
 * @file API_debounce.c
 * @brief Implementación de una máquina de estados finitos para el debounce de un botón utilizando retardos no bloqueantes.
 *
 * Creado como parte de la práctica N° 4 de `Programación de Microcontroladores`
 *
 * @author Jez
 * @date Mar 27, 2025
 */


#include <API_debounce.h>

#define DEBOUNCE_TIME 40
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA


static debounceState_t FSMstate; // estado de la FSM
static delay_t debounceDelay;
static bool_t buttonWasPressed = false;

/**
 * @brief Callback que se ejecuta cuando se detecta una pulsación del botón.
 *
 * Marca la variable `buttonWasPressed` como verdadera para indicar que el botón fue presionado.
 */
static void buttonPressed(void);

void debounceFSM_init(void){
	FSMstate = BUTTON_UP;
	delayInit(&debounceDelay, DEBOUNCE_TIME);
}

buttonChange_t debounceFSM_update(){
	buttonChange_t bChange;
	bChange = BUTTON_UNCHANGED;
	switch(FSMstate){
	case BUTTON_UP:
		if(!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)){ // si detecto un 0 (posible presion de boton - boton con pullup resistor)
			FSMstate = BUTTON_FALLING;
			delayRead(&debounceDelay); // pongo a correr el timer
		}
		bChange = BUTTON_UNCHANGED;
		break;
	case BUTTON_FALLING:
		if(delayRead(&debounceDelay)){ // si termino el delay
			if(!HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)){ // efectivamente esta presionado
				FSMstate = BUTTON_DOWN;
				buttonPressed();
				bChange = BUTTON_PRESSED;
			} else { // fue ruido
				FSMstate = BUTTON_UP;
				bChange = BUTTON_NOISE_DETECTED;
			}
		}
		break;
	case BUTTON_DOWN:
		if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)){ // si detecto un 1 (posible release del boton - boton con pullup resistor)
			FSMstate = BUTTON_RISING;
			delayRead(&debounceDelay); // pongo a correr el timer
		}
		bChange = BUTTON_UNCHANGED;
		break;
	case BUTTON_RISING:
		if(delayRead(&debounceDelay)){// si termino el delay
			if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)){ // efectivamente se solto el boton
				FSMstate = BUTTON_UP;
				bChange = BUTTON_RELEASED;
			} else { // fue ruido
				FSMstate = BUTTON_DOWN;
				bChange = BUTTON_NOISE_DETECTED;
			}
		}
		break;
	default:
		debounceFSM_init();
		bChange = BUTTON_ERROR;
	}
	return bChange;
}

bool_t readKey(void){
	bool_t returnVal = buttonWasPressed;
	buttonWasPressed = false;
	return returnVal;
}

static void buttonPressed(void){
	buttonWasPressed = true;
}
//void static buttonReleased(void){
	// no implementada
//}
