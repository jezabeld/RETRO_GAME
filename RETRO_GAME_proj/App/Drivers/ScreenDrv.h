/*
 * ScreenDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_SCREENDRV_H_
#define DRIVERS_SCREENDRV_H_

#include <stdint.h>
#include <stdbool.h>

int tftInit(void);
int tftSetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
int tftDrawImageDMA(const uint8_t* imageData, uint32_t size);

#endif /* DRIVERS_SCREENDRV_H_ */