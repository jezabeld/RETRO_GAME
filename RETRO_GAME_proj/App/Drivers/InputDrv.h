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

typedef struct {
    uint16_t jyX;  // X axis (PA1)
    uint16_t jyY;  // Y axis (PA0)
} joystick_t;

uint8_t inputInit();
uint8_t inputGetJoyAxis(uint8_t axis, joystick_t* joyPtr);  // axis: 0=X, 1=Y. Returns: 0=success, 1=error
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void btnAtimerCallback(TimerHandle_t hTimer);
void btnBtimerCallback(TimerHandle_t hTimer);
void btnCtimerCallback(TimerHandle_t hTimer);
void btnDtimerCallback(TimerHandle_t hTimer);

#endif /* DRIVERS_INPUTDRV_H_ */
