/*
 * HapticDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_HAPTICDRV_H_
#define DRIVERS_HAPTICDRV_H_

#include <stdint.h>
#include <stdbool.h>

int hapticInit(void);
int hapticPlay(uint8_t effect_id);

#endif /* DRIVERS_HAPTICDRV_H_ */