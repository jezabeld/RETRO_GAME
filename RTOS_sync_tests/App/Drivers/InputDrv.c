/*
 * InputDrv.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "InputDrv.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "events.h"
#include "cmsis_os.h"

extern TimerHandle_t tBtnAdebounce;
extern TimerHandle_t tBtnBdebounce;
extern TimerHandle_t tBtnCdebounce;
extern TimerHandle_t tBtnDdebounce;

int inputInit(const input_hw_cfg_t *cfg)
{
    return 0;
}
void EXTI_DisableLine(uint32_t line_bit) { EXTI->IMR &= ~line_bit; }
void EXTI_EnableLine (uint32_t line_bit) { EXTI->IMR |=  line_bit; }
void EXTI_ClearPending(uint32_t line_bit){ EXTI->PR   =  line_bit; } // write-1-to-clear

/*-----------------------------------------------------------------------
 * Callback común de los 4 botones (PC0…PC3)
 *---------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	event_id_t inpEvent;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	switch (GPIO_Pin) {
	case BTN_A_Pin:
	    inpEvent = RAW_BTN_A;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
	    xTimerStartFromISR(tBtnAdebounce,&xHigherPriorityTaskWoken);
	    HAL_NVIC_DisableIRQ(BTN_A_EXTI_IRQn);
	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_B_Pin:
	    inpEvent = RAW_BTN_B;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
	    xTimerStartFromISR(tBtnBdebounce,&xHigherPriorityTaskWoken);
	    HAL_NVIC_DisableIRQ(BTN_B_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_C_Pin:
	    inpEvent = RAW_BTN_C;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
	    xTimerStartFromISR(tBtnCdebounce,&xHigherPriorityTaskWoken);
	    HAL_NVIC_DisableIRQ(BTN_C_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_D_Pin:
	    inpEvent = RAW_BTN_D;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
	    xTimerStartFromISR(tBtnDdebounce,&xHigherPriorityTaskWoken);
	    HAL_NVIC_DisableIRQ(BTN_D_EXTI_IRQn);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	default: return;                     /* otro pin, ignorar */
	}
}
