/*
 * @file GY521.c
 * @brief Device driver para el acelerómetro del módulo GY-521 con controlador MPU-6500.
 *
 * Creado como parte del Trabajo Práctico final de las materias `Protocolos de
 * 	Comunicación en Sistemas embebidos` y `Programación de Microcontroladores`.
 *
 * @author  Jez
 * @date Apr 6, 2025
 */

#include "GY521.h"
#include "assert.h"
#include "API_uart.h"

// Registros internos del GY-521/MPU-6500
static const uint8_t REG_WHO_AM_I = 0x75;
//static const uint8_t REG_CONFIG = 0x1A;
//static const uint8_t REG_GYRO_CONFIG = 0x1B;
static const uint8_t REG_ACCEL_CONFIG = 0x1C;
static const uint8_t REG_ACCEL_CONFIG_2 = 0x1D;
static const uint8_t REG_LPA_ODR = 0x1E;
static const uint8_t REG_PWR_MGMT_1 = 0x6B;
static const uint8_t REG_PWR_MGMT_2 = 0x6C;
static const uint8_t REG_ACCEL_XOUT_H = 0x3B; ///< Registers 0x3B to 0x40 – Accelerometer Measurements

static const uint8_t DEV_ID = 0x70; 		///< device ID for MPU-6500
static const uint8_t CALIBRATION_SAMPLES = 100;
static const uint8_t MIN_MEASURE_TIME = 10; ///< Min time in millisecs between calibration measurements.

// Valores de control
#define PWR_MGMT 0x00
#define DEVICE_RESET (1<<7)
#define CYCLE (1<<5)
#define TEMP_DIS (1<<3) ///< Disable temp sensor.
#define CLKSEL_INTERNAL 0
#define CLSEL_AUTO 1
#define GYRO_OFF 7		///< Disable gyroscope.
#define ACCEL_OFF (7<<3)
#define LP_WAKE_CTRL (3<<6) ///< Freq for wake-up and sample in LOW_POWER_ACC_MODE.
#define READ1BYTE 1
#define READ6BYTES 6
#define ACC_DLPF 2
// accelerometer values offset in register read
#define ACC_XH 0
#define ACC_XL 1
#define ACC_YH 2
#define ACC_YL 3
#define ACC_ZH 4
#define ACC_ZL 5

/**
 * @brief Calibra los 3 ejes del acelerómetro acorde a la posición actual.
 *
 * @param[in,out] gyro Estructura del dispostivo (debe haber sido previamente inicializado).
 */
static void calibrateAccel(gyro_t * gyro);

gyroStatus_t gyroInit(gyro_t * gyro, I2C_HandleTypeDef * hi2c, uint8_t devAddress, gyroPowerModes_t mode){
	assert(gyro);
	assert(hi2c);
	assert_param(devAddress); // check address is not 0

	gyro->hi2c = hi2c;
	gyro->devAddress = devAddress;
	gyro->calX = 0;
	gyro->calY = 0;
	gyro->calZ = 0;

	// test connection
	bool status; // variable de estatus de comunicacion
	uint8_t check;
	status = readRegister(gyro->hi2c, gyro->devAddress, &check, REG_WHO_AM_I, READ1BYTE);
	if(status || (check != DEV_ID)) return GYRO_ERROR;

	// set power management
	uint8_t pwrMgmt;

	switch(mode){
	case LOW_POWER_ACC_MODE:
		pwrMgmt = PWR_MGMT + TEMP_DIS + CYCLE + CLKSEL_INTERNAL;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_PWR_MGMT_1);

		pwrMgmt = PWR_MGMT + GYRO_OFF + LP_WAKE_CTRL;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_PWR_MGMT_2);

		pwrMgmt = PWR_MGMT;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_LPA_ODR);

		break;
	case LOW_NOISE_ACC_MODE:
		pwrMgmt = PWR_MGMT + TEMP_DIS + CLKSEL_INTERNAL;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_PWR_MGMT_1);

		pwrMgmt = PWR_MGMT + GYRO_OFF;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_PWR_MGMT_2);

		pwrMgmt = PWR_MGMT;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_ACCEL_CONFIG);

		pwrMgmt = PWR_MGMT + ACC_DLPF;
		status |= writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_ACCEL_CONFIG_2);

		break;
	default:
		return GYRO_ERROR;
	}

	// Calibración inicial del dispositivo.
	calibrateAccel(gyro);

	return (status? GYRO_ERROR : GYRO_OK);
}

gyroStatus_t gyroReadAccel(gyro_t * gyro, int16_t * accX, int16_t * accY, int16_t * accZ){
	assert(gyro);
	assert(gyro->hi2c); // Verifica que la estructura fue inicializada

	uint8_t values[READ6BYTES];
	// Los 6 registros del acelerómetro son contiguos y pueden ser leídos en una única comunicación
	bool status = readRegister(gyro->hi2c, gyro->devAddress, values, REG_ACCEL_XOUT_H, READ6BYTES);
	*accX = (int16_t)((values[ACC_XH]<<8) | (values[ACC_XL])) - gyro->calX;
	*accY = (int16_t)((values[ACC_YH]<<8) | (values[ACC_YL])) - gyro->calY;
	*accZ = (int16_t)((values[ACC_ZH]<<8) | (values[ACC_ZL])) - gyro->calZ;

	return (status? GYRO_ERROR : GYRO_OK);
}

static void calibrateAccel(gyro_t * gyro){
	assert(gyro);
	assert(gyro->hi2c); // Verifica que la estructura fue inicializada

	int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    int16_t accX, accY, accZ;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        gyroReadAccel(gyro, &accX, &accY, &accZ);
        sum_x += accX;
        sum_y += accY;
        sum_z += accZ;
        HAL_Delay(MIN_MEASURE_TIME);
    }

    gyro->calX = sum_x / CALIBRATION_SAMPLES;
    gyro->calY = sum_y / CALIBRATION_SAMPLES;
    gyro->calZ = (sum_z / CALIBRATION_SAMPLES) - _1G;
}
