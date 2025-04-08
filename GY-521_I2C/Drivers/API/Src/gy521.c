/*
 * gy521.c
 *
 *  Created on: Apr 6, 2025
 *      Author: jez
 */

#include "gy521.h"

#define PWR_MGMT 0x00
#define DEVICE_RESET 0x01
#define CYCLE (1<<2)
#define TEMP_DIS (1<<4) // disable temp sensor
#define CLKSEL_INTERNAL 0
#define GYRO_OFF (7<<5)
#define LP_WAKE_CTRL 3 // freq for wake-up and sample
#define READ1BYTE 1
#define READ6BYTES 6
// accelerometer values offset in register read
#define ACC_XH 0
#define ACC_XL 1
#define ACC_YH 2
#define ACC_YL 3
#define ACC_ZH 4
#define ACC_ZL 5


// Registros internos del GY-521
static const uint8_t REG_WHO_AM_I = 0x75;
static const uint8_t REG_CONFIG = 0x1A;
static const uint8_t REG_GYRO_CONFIG = 0x1B;
static const uint8_t REG_ACCEL_CONFIG = 0x1C;
static const uint8_t REG_PWR_MGMT_1 = 0x6B;
static const uint8_t REG_PWR_MGMT_2 = 0x6C;
static const uint8_t REG_ACCEL_XOUT_H = 0x3B; // Registers 0x3B to 0x40 – Accelerometer Measurements

static const uint8_t DEV_ID = 0x70; // device ID for MPU-6500


bool_t gyroInit(gyro_t * gyro, I2C_HandleTypeDef * hi2c, uint8_t devAddress){
	gyro->hi2c = hi2c;
	gyro->devAddress = devAddress;

	// set power management: Low-Power Accelerometer Mode
	uint8_t pwrMgmt = PWR_MGMT + TEMP_DIS + CYCLE + CLKSEL_INTERNAL;
	writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_PWR_MGMT_1);

	pwrMgmt = PWR_MGMT + GYRO_OFF + LP_WAKE_CTRL;
	writeRegister(gyro->hi2c, gyro->devAddress, &pwrMgmt, REG_PWR_MGMT_2);

	// test connection
	uint8_t check;
	readRegister(gyro->hi2c, gyro->devAddress, &check, REG_WHO_AM_I, READ1BYTE);
	if (check != DEV_ID){
		return false;
	}
	return true;
}

void gyroReadAccel(gyro_t * gyro, int16_t * accX, int16_t * accY, int16_t * accZ){
	uint8_t values[READ6BYTES];
	readRegister(gyro->hi2c, gyro->devAddress, values, REG_ACCEL_XOUT_H, READ6BYTES);
	*accX = (int16_t)((values[ACC_XH]<<8) | (values[ACC_XL]));
	*accY = (int16_t)((values[ACC_YH]<<8) | (values[ACC_YL]));
	*accZ = (int16_t)((values[ACC_ZH]<<8) | (values[ACC_ZL]));
}
