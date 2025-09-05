/*
 * InputDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_INPUTDRV_H_
#define DRIVERS_INPUTDRV_H_

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

uint8_t inputInit();
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void btnAtimerCallback(TimerHandle_t hTimer);
void btnBtimerCallback(TimerHandle_t hTimer);
void btnCtimerCallback(TimerHandle_t hTimer);
void btnDtimerCallback(TimerHandle_t hTimer);

#endif /* DRIVERS_INPUTDRV_H_ */
