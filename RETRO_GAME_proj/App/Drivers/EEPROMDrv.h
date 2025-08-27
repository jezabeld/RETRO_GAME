/*
 * EEPROMDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_EEPROMDRV_H_
#define DRIVERS_EEPROMDRV_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

int eepromDrvInit(void);
int eepromRead(uint32_t addr, uint8_t* buf, size_t len);
int eepromWrite(uint32_t addr, const uint8_t* buf, size_t len);

#endif /* DRIVERS_EEPROMDRV_H_ */