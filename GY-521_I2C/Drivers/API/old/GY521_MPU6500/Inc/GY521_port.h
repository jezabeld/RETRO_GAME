/*
 * @file GY521_port.h
 * @brief Device driver para el acelerómetro del módulo GY-521 con controlador MPU-6500. Funciones para el hardware específico.
 *
 * Creado como parte del Trabajo Práctico final de las materias `Protocolos de
 * 	Comunicación en Sistemas embebidos` y `Programación de Microcontroladores`.
 *
 * @author  Jez
 * @date Apr 6, 2025
 */

#ifndef API_INC_API_GY521_PORT_H_
#define API_INC_API_GY521_PORT_H_

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

/**
 * @brief Lee un registro (o un conjunto de registros contiguos) del dispositivo conectado por I2C en la dirección seleccionada.
 *
 * @param[in] hi2c Handler de I2C para la conexión con el dispositivo.
 * @param devAddress Dirección del dispositivo para la conexión por I2C (en formato 7 bits).
 * @param[out] data Puntero donde guardar la información leída del registro.
 * @param regAddress Dirección del registro a leer (en formato 7 bits).
 * @param sizeRead Tamaño a leer del registro seleccionado en bytes. Valores válidos entre 1 y 14 bytes.
 * @return `false` si la comunicación I2C fue exitosa, `true` en caso contrario.
 */
bool readRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress, uint8_t sizeRead);

/**
 * @brief Escribe un registro del dispositivo conectado por I2C en la dirección seleccionada.
 *
 * @param[in] hi2c Handler de I2C para la conexión con el dispositivo.
 * @param devAddress Dirección del dispositivo para la conexión por I2C (en formato 7 bits).
 * @param[in] data Puntero donde se encuentra la información a escribir en el registro.
 * @param regAddress Dirección del registro a escribir (en formato 7 bits).
 * @return `false` si la comunicación I2C fue exitosa, `true` en caso contrario.
 */
bool writeRegister(I2C_HandleTypeDef * hi2c, uint8_t devAddress, uint8_t * data, uint8_t regAddress);

#endif /* API_INC_API_GY521_PORT_H_ */
