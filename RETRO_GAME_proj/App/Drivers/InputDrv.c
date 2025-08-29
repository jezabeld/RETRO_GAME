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

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
volatile uint16_t joy_raw[2] = { 0 }; /* [0]=PA1 X , [1]=PA0 Y */

int inputInit(const input_hw_cfg_t *cfg)
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joy_raw, 2);
    return 0;
}

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
	    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_B_Pin:
	    inpEvent = RAW_BTN_B;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_C_Pin:
	    inpEvent = RAW_BTN_C;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	case BTN_D_Pin:
	    inpEvent = RAW_BTN_D;
	    xQueueSendFromISR(qEvents, &inpEvent, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	default: return;                     /* otro pin, ignorar */
	}
}
