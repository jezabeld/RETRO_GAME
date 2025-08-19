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

typedef void (*timer_cb_t)(void);

int timerInitLvgl(void);
int timerRegister(timer_cb_t cb, uint32_t period_ms);

#endif /* DRIVERS_TIMERDRV_H_ */