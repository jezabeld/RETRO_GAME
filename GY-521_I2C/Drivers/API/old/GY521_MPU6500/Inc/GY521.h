/*
 * @file GY521.h
 * @brief Device driver para el acelerómetro del módulo GY-521 con controlador MPU-6500.
 *
 * Creado como parte del Trabajo Práctico final de las materias `Protocolos de
 * 	Comunicación en Sistemas embebidos` y `Programación de Microcontroladores`.
 *
 * @author  Jez
 * @date Apr 6, 2025
 */

#ifndef API_INC_API_GY521_H_
#define API_INC_API_GY521_H_

#include "GY521_port.h"

#define _1G 16384 ///< Valor de lectura correspondiente a 1G en modo ±2G
#define _2G 32768 ///< Valor de lectura correspondiente a 2G en modo ±2G

/**
 * @typedef gyro_t
 * @brief Estructura del giroscopio/acelerometro.
 */
typedef struct{
   I2C_HandleTypeDef * hi2c;
   uint8_t devAddress;
   int16_t calX;
   int16_t calY;
   int16_t calZ;
} gyro_t;

/**
 * @brief Modos de alimentación y funcionamiento del dispositivo.
 *
 * Implementados únicamente los modos de acelerómetro para este driver.
 */
typedef enum
{
//	SLEEP_MODE 	= 1,	///< Modos no implementados en este driver.
//	STANDBY_MODE,
	LOW_POWER_ACC_MODE,
	LOW_NOISE_ACC_MODE,
//	GYR_MODE,
//	FULL_MODE,
} gyroPowerModes_t;

/**
 * @brief Estados del dispositivo.
 */
typedef enum{
	GYRO_ERROR,
	GYRO_OK,
} gyroStatus_t;

/**
 * @brief Inicializa el giroscopio y acelerometro en el modo de alimentación seleccionado.
 *
 * Realiza la inicialización del dispositivo en el modo de alimentación seleccionado, testea la correcta
 * comunicación con el dispositivo y realiza una calibración inicial del mismo.
 *
 * @param[in,out] gyro Estructura del dispostivo a inicializar.
 * @param[in] hi2c Handler de I2C para la conexión con el dispositivo.
 * @param devAddress Dirección del dispositivo para la conexión por I2C.
 * @param mode Modo de alimentación y funcionamiento seleccionado.
 * @return Estado de la inicialización: `GYRO_OK` si la incialización finalizó con éxito,
 * 			`GYRO_ERROR` si hubo problemas de conexión o comunicación con el dispositivo o el modo de
 * 			alimentación seleccionado es incorrecto o no definido. `GYRO_ERROR` También puede indicar
 * 			errores de comunicación con el dispositvo debido a una dirección errónea del mismo o que el
 * 			dispositivo conectado no es el modelo MPU-6500. Otros modelos requieren configuraciones
 * 			diferentes y no se recomienda el uso de este driver.
 */
gyroStatus_t gyroInit(gyro_t * gyro, I2C_HandleTypeDef * hi2c, uint8_t devAddress, gyroPowerModes_t mode);

/**
 * @brief Lee los valores del acelerómetro.
 *
 * @param[in] gyro Estructura del dispostivo (debe haber sido previamente inicializado).
 * @param[out] accX Puntero a la direccion de memoria donde se guardará el valor leído del eje X.
 * @param[out] accY Puntero a la direccion de memoria donde se guardará el valor leído del eje Y.
 * @param[out] accZ Puntero a la direccion de memoria donde se guardará el valor leído del eje Z.
 * @return `GYRO_OK` si la lectura se realizó correctamente, `GYRO_ERROR` en caso contrario.
 */
gyroStatus_t gyroReadAccel(gyro_t * gyro, int16_t * accX, int16_t * accY, int16_t * accZ);

#endif /* API_INC_API_GY521_H_ */

