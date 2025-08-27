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

typedef struct {
    uint8_t buttonCount;
    uint8_t joystickChannels;
} input_hw_cfg_t;

int inputInit(const input_hw_cfg_t *cfg);

#endif /* DRIVERS_INPUTDRV_H_ */