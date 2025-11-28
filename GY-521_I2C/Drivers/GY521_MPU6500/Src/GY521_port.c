/*
 * @file GY521_port.c
 * @brief Device driver para el acelerómetro del módulo GY-521 con controlador MPU-6500. Funciones para el hardware específico.
 *
 * Creado como parte del Trabajo Práctico final de las materias `Protocolos de
 * 	Comunicación en Sistemas embebidos` y `Programación de Microcontroladores`.
 *
 * @author  Jez
 * @date Apr 6, 2025
 */

#include "GY521_port.h"
#include "assert.h"

#define MAX_U7B 127 		///< SPI address de 7 bits
#define MAX_READ_BYTES 14	///< lectura de 6 valores de acelerometro, 6 de giroscopio y 2 de temperatura

bool readRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress, uint8_t sizeRead){
	assert(hi2c);
	assert(data);
	assert_param(devAddress > 0 && devAddress <= MAX_U7B);
	assert_param(regAddress >= 0 && regAddress < MAX_U7B);
	assert_param(sizeRead > 0 && sizeRead < MAX_READ_BYTES);

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, devAddress<<1, regAddress, 1, data, sizeRead, HAL_MAX_DELAY);
	if(status != HAL_OK){
		return true;
	}
	return false;
}

bool writeRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress){
	assert(hi2c);
	assert(data);
	assert_param(devAddress > 0 && devAddress <= MAX_U7B);
	assert_param(regAddress >= 0 && regAddress < MAX_U7B);

	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c, devAddress<<1, regAddress, 1, data, 1, HAL_MAX_DELAY);
	if(status != HAL_OK){
		return true;
	}
	return false;
}
