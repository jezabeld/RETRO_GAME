/*
 * gy521_port.h
 *
 *  Created on: Apr 6, 2025
 *      Author: jez
 */

#ifndef API_INC_GY521_PORT_H_
#define API_INC_GY521_PORT_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"

void readRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress, uint8_t sizeRead);

void writeRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress);

#endif /* API_INC_GY521_PORT_H_ */
