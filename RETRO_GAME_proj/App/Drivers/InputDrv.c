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

extern TimerHandle_t tBtnAdebounce;
extern TimerHandle_t tBtnBdebounce;
extern TimerHandle_t tBtnCdebounce;
extern TimerHandle_t tBtnDdebounce;

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
volatile uint16_t joy_raw[2] = { 0 }; /* [0]=PA1 X , [1]=PA0 Y */

uint8_t inputInit()
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joy_raw, 2);

    return 0;
}

void btnAtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_A_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_A_EXTI_IRQn);
	uartSendString("btnAtimerCallback\r\n");
	event_id_t inpEvent = INP_BTN_A;
	xQueueSend(qEvents, &inpEvent, 0);
}
void btnBtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_B_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_B_EXTI_IRQn);
	uartSendString("btnBtimerCallback\r\n");
	event_id_t inpEvent = INP_BTN_B;
	xQueueSend(qEvents, &inpEvent, 0);
}
void btnCtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_C_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_C_EXTI_IRQn);
	uartSendString("btnCtimerCallback\r\n");
	event_id_t inpEvent = INP_BTN_C;
	xQueueSend(qEvents, &inpEvent, 0);
}
void btnDtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_D_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_D_EXTI_IRQn);
	uartSendString("btnDtimerCallback\r\n");
	event_id_t inpEvent = INP_BTN_D;
	xQueueSend(qEvents, &inpEvent, 0);
}
/*-----------------------------------------------------------------------
 * Callback común de los 4 botones (PC0…PC3)
 *---------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	event_id_t inpEvent;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	switch (GPIO_Pin) {
	case BTN_A_Pin:
//		inpEvent = RAW_BTN_A;
//		xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		xTimerStartFromISR(tBtnAdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_A_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_B_Pin:
//		inpEvent = RAW_BTN_B;
//		xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		xTimerStartFromISR(tBtnBdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_B_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_C_Pin:
//		inpEvent = RAW_BTN_C;
//		xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		xTimerStartFromISR(tBtnCdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_C_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_D_Pin:
//		inpEvent = RAW_BTN_D;
//		xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		xTimerStartFromISR(tBtnDdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_D_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	default: return;                     /* otro pin, ignorar */
	}
}

