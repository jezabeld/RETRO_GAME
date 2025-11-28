/*
 * gy521_port.c
 *
 *  Created on: Apr 6, 2025
 *      Author: jez
 */

#include "gy521_port.h"





void sendCommand(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t value){
	HAL_I2C_Master_Transmit(hi2c, devAddress<<1, &value, sizeof(value),HAL_MAX_DELAY);
}

void readRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress, uint8_t sizeRead){
	HAL_I2C_Mem_Read(hi2c, devAddress<<1, regAddress, 1, data, sizeRead, HAL_MAX_DELAY);
}

void writeRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress){
	HAL_I2C_Mem_Write(hi2c, devAddress<<1, regAddress, 1, data, 1, HAL_MAX_DELAY);
}
