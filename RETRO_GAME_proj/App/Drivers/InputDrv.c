/*
 * InputDrv.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "InputDrv.h"
#include "stm32f4xx_hal.h"
#include "synchronization.h"
#include "main.h"
#include "UARTDrv.h"

// ==== Parámetros ====
#define DEBOUNCE_MS   25      // 20–30 ms suele ir bien
#define ACTIVE_LOW    1       // botones con pull-up → nivel bajo = pulsado

// ==== Estados del botón ====
typedef enum {
    BTN_IDLE,           // No presionado, esperando flanco descendente
    BTN_PRESS_PENDING,  // Flanco detectado, debouncing en curso  
    BTN_PRESSED         // Press confirmado (futuro: hold/release aquí)
} btn_state_t;

// ==== Información por botón ====
typedef struct {
    btn_state_t state;          // Estado actual
    GPIO_TypeDef* port;         // Puerto GPIO
    uint16_t pin;               // Pin GPIO
    IRQn_Type irqn;             // IRQ number
    uint32_t debounce_start;    // Timestamp cuando empezó debounce
    event_id_t press_event;     // Evento a enviar
    TimerHandle_t timer;        // Timer individual (mantenemos por compatibilidad)
} btn_info_t;

extern TimerHandle_t tBtnAdebounce;
extern TimerHandle_t tBtnBdebounce;
extern TimerHandle_t tBtnCdebounce;
extern TimerHandle_t tBtnDdebounce;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
volatile uint16_t joy_raw[2] = { 0 }; /* [0]=PA1 X , [1]=PA0 Y */

// ==== Array de información de botones ====
static btn_info_t s_buttons[4] = {
    {BTN_IDLE, BTN_A_GPIO_Port, BTN_A_Pin, BTN_A_EXTI_IRQn, 0, INP_BTN_A, NULL}, // BTN_A
    {BTN_IDLE, BTN_B_GPIO_Port, BTN_B_Pin, BTN_B_EXTI_IRQn, 0, INP_BTN_B, NULL}, // BTN_B
    {BTN_IDLE, BTN_C_GPIO_Port, BTN_C_Pin, BTN_C_EXTI_IRQn, 0, INP_BTN_C, NULL}, // BTN_C
    {BTN_IDLE, BTN_D_GPIO_Port, BTN_D_Pin, BTN_D_EXTI_IRQn, 0, INP_BTN_D, NULL}  // BTN_D
};

// ==== Funciones helper ====
static void handleButtonDebounce(btn_info_t* btn, const char* btn_name) {
    // Máquina de estados: verificar si el botón sigue presionado
    if (btn->state == BTN_PRESS_PENDING) {
      // Leer el estado actual del pin
      GPIO_PinState pin_state = HAL_GPIO_ReadPin(btn->port, btn->pin);

      // Verificar si está presionado considerando ACTIVE_LOW
      uint8_t is_pressed = (ACTIVE_LOW) ? (pin_state == GPIO_PIN_RESET) : (pin_state == GPIO_PIN_SET);

      if (is_pressed) {
        btn->state = BTN_PRESSED;
        uartSendString(btn_name);
        uartSendString(" PRESSED\r\n");
        // Enviar evento de press confirmado
        xQueueSend(qEvents, &(btn->press_event), 0);
      } else {
        // Falsa alarma, botón no está presionado
        btn->state = BTN_IDLE;
        uartSendString(btn_name);
        uartSendString(" false alarm\r\n");
      }
    }
    
    // Limpia cualquier pending y re-habilita IRQ
    __HAL_GPIO_EXTI_CLEAR_IT(btn->irqn);
    HAL_NVIC_EnableIRQ(btn->irqn);
    btn->state = BTN_IDLE;  // Reset para próximo press
}

uint8_t inputInit()
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joy_raw, 2);

    // Inicializar referencias a timers existentes (mantenemos compatibilidad)
    s_buttons[0].timer = tBtnAdebounce;  // BTN_A
    s_buttons[1].timer = tBtnBdebounce;  // BTN_B
    s_buttons[2].timer = tBtnCdebounce;  // BTN_C  
    s_buttons[3].timer = tBtnDdebounce;  // BTN_D

    return 0;
}

void btnAtimerCallback(TimerHandle_t hTimer){
	handleButtonDebounce(&s_buttons[0], "btnA");
}
void btnBtimerCallback(TimerHandle_t hTimer){
	handleButtonDebounce(&s_buttons[1], "btnB");
}
void btnCtimerCallback(TimerHandle_t hTimer){
	handleButtonDebounce(&s_buttons[2], "btnC");
}
void btnDtimerCallback(TimerHandle_t hTimer){
	handleButtonDebounce(&s_buttons[3], "btnD");
}
/*-----------------------------------------------------------------------
 * Callback común de los 4 botones (PC0…PC3)
 *---------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	switch (GPIO_Pin) {
	case BTN_A_Pin:
		s_buttons[0].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnAdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_A_EXTI_IRQn);
		break;
	case BTN_B_Pin:
		s_buttons[1].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnBdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_B_EXTI_IRQn);
		break;
	case BTN_C_Pin:
		s_buttons[2].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnCdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_C_EXTI_IRQn);
		break;
	case BTN_D_Pin:
		s_buttons[3].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnDdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_D_EXTI_IRQn);
		break;
	default: 
		return;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

