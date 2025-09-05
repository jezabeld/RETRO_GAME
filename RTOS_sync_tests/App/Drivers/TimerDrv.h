/*
 * TimerDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_TIMERDRV_H_
#define DRIVERS_TIMERDRV_H_

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

uint8_t timerInit(void);
void btnAtimerCallback(TimerHandle_t hTimer);
void btnBtimerCallback(TimerHandle_t hTimer);
void btnCtimerCallback(TimerHandle_t hTimer);
void btnDtimerCallback(TimerHandle_t hTimer);

#endif /* DRIVERS_TIMERDRV_H_ */
