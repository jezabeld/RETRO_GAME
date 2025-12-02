/**
 * @file AccelDrv.h
 * @brief Complete MPU6050 accelerometer driver with I2C DMA continuous reading
 *
 * Full initialization, calibration, and timer-triggered I2C DMA reads.
 * 
 * @date Nov 28 2025
 * @author jez
 */

#ifndef DRIVERS_ACCELDRV_H_
#define DRIVERS_ACCELDRV_H_

/* === Headers files inclusions ================================================================ */
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* === Public macros definitions =============================================================== */
#define _1G 16384  ///< Value corresponding to 1G in ±2G range

/* === Public data type declarations =========================================================== */

/**
 * @brief MPU6050 power modes
 */
typedef enum {
    ACCEL_LOW_POWER_MODE,  ///< Low power accelerometer mode
    ACCEL_LOW_NOISE_MODE   ///< Low noise accelerometer mode 
} accelPowerMode_t;

/**
 * @brief Driver status
 */
typedef enum {
    ACCEL_ERROR = 0,
    ACCEL_OK = 1
} accelStatus_t;

/**
 * @brief Accelerometer tilt data structure (normalized values)
 */
typedef struct {
    int16_t tiltX; ///< X-axis tilt, normalized to -1024..+1023 (±1G ≈ ±1024, 2048 values)
    int16_t tiltY; ///< Y-axis tilt, normalized to -1024..+1023 (±1G ≈ ±1024, 2048 values)
} accelTilt_t;

/* === Public function declarations ============================================================ */

/**
 * @brief Initialize MPU6050 and start continuous DMA reading
 *
 * Performs full initialization sequence:
 * - Verifies MPU6050 device ID
 * - Configures power management and accelerometer settings
 * - Performs calibration (100 samples)
 * - Starts timer-triggered I2C DMA reads
 *
 * @param i2cHandle Pointer to I2C handle (must have DMA configured)
 * @param timHandle Pointer to timer for periodic triggering (e.g., 100Hz)
 * @param devAddress MPU6050 I2C address (typically 0x68)
 * @param mode Power mode (ACCEL_LOW_NOISE_MODE recommended)
 * @return ACCEL_OK on success, ACCEL_ERROR on failure
 *
 * @note Calibration assumes device is stationary and level
 * @note I2C DMA RX must be configured in CubeMX
 * @note Timer should be configured for desired sample rate (50-200Hz)
 */
accelStatus_t accelDrvInit(I2C_HandleTypeDef *i2cHandle,
                           TIM_HandleTypeDef *timHandle,
                           uint8_t devAddress,
                           accelPowerMode_t mode);

/**
 * @brief Get latest filtered tilt values (normalized)
 *
 * Thread-safe, non-blocking read of last processed accelerometer data.
 * Values are calibrated (centered at 0), filtered with EMA + deadband,
 * and normalized to -1024..+1023 range (2048 total values).
 *
 * @param tilt Pointer to structure to receive tilt data
 *
 * @note Values normalized: -1024..+1023 range, ±1024 ≈ ±1G tilt
 * @note Deadband applied before normalization: ±100 raw counts ≈ ±0.006G
 * @note Hardware-independent output, suitable for direct use in game logic
 */
void accelDrvGetTilt(accelTilt_t *tilt);

#endif /* DRIVERS_ACCELDRV_H_ */