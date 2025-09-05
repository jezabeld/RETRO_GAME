/*
 * TimerDrv.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "TimerDrv.h"
#include "main.h"
#include "UARTDrv.h"

extern TimerHandle_t tBtnAdebounce;
extern TimerHandle_t tBtnBdebounce;
extern TimerHandle_t tBtnCdebounce;
extern TimerHandle_t tBtnDdebounce;

uint8_t timerInit(void){
	return 0u;
}

void btnAtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_A_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_A_EXTI_IRQn);
	uartSendString("btnAtimerCallback\r\n");
}
void btnBtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_B_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_B_EXTI_IRQn);
	uartSendString("btnBtimerCallback\r\n");
}
void btnCtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_C_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_C_EXTI_IRQn);
	uartSendString("btnCtimerCallback\r\n");
}
void btnDtimerCallback(TimerHandle_t hTimer){
	// Limpia cualquier pending acumulado durante la ventana…
	__HAL_GPIO_EXTI_CLEAR_IT(BTN_D_EXTI_IRQn);
	// …y re-habilita SOLO su línea
	HAL_NVIC_EnableIRQ(BTN_D_EXTI_IRQn);
	uartSendString("btnDtimerCallback\r\n");
}
