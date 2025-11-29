/**
 * @file AccelJoystickDrv.c
 * @brief Complete MPU6050 accelerometer driver with I2C DMA continuous reading
 *
 * Full initialization, calibration, and timer-triggered I2C DMA reads.
 * No dedicated task required - saves 512-1024 bytes of stack.
 *
 * @date Nov 28 2025
 * @author jez
 */

/* === Headers files inclusions ================================================================ */
#include "AccelJoystickDrv.h"

/* === Macros definitions ====================================================================== */
// MPU6050 Register addresses
#define REG_WHO_AM_I 0x75
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_CONFIG_2 0x1D
#define REG_LPA_ODR 0x1E
#define REG_PWR_MGMT_1 0x6B
#define REG_PWR_MGMT_2 0x6C
#define REG_ACCEL_XOUT_H 0x3B

#define DEV_ID 0x70                    ///< Device ID for MPU-6500
#define CALIBRATION_SAMPLES 100        ///< Number of samples for calibration
#define MIN_MEASURE_TIME 10            ///< Min time in ms between calibration samples

// Power management values
#define PWR_MGMT 0x00
#define DEVICE_RESET (1<<7)
#define CYCLE (1<<5)
#define TEMP_DIS (1<<3)
#define CLKSEL_INTERNAL 0
#define CLSEL_AUTO 1
#define GYRO_OFF 7
#define ACCEL_OFF (7<<3)
#define LP_WAKE_CTRL (3<<6)
#define ACC_DLPF 2

// Filtering parameters
#define ALPHA_Q15 16384u               ///< EMA filter alpha (≈0.5 for stability)
#define DEADBAND 100                   ///< Center deadband (±100 ≈ ±0.006G)
#define DMA_TIMEOUT_TICKS 5            ///< Max timer ticks to wait for DMA (50ms @100Hz)

/* === Private data type declarations ========================================================== */

/**
 * @brief I2C DMA buffer for 6-byte accelerometer read
 */
typedef struct {
    uint8_t buff[6];           ///< DMA buffer: XH,XL,YH,YL,ZH,ZL
    volatile uint8_t busy;     ///< Transfer in progress flag
    volatile uint8_t busyTicks; ///< Ticks counter while busy (watchdog)
} i2cBuffer_t;

/**
 * @brief Calibration and filter state
 */
typedef struct {
    int16_t calX, calY, calZ;  ///< Calibration offsets
    int32_t emaX, emaY;        ///< EMA accumulators
} filterState_t;

/* === Private variable definitions ============================================================ */

static i2cBuffer_t dmaBuffer = { .buff = {0}, .busy = 0, .busyTicks = 0 };
static filterState_t state = { .calX = 0, .calY = 0, .calZ = 0, .emaX = 0, .emaY = 0 };
static volatile accelTilt_t published = { .tiltX = 0, .tiltY = 0 };

static I2C_HandleTypeDef *pI2c = NULL;
static TIM_HandleTypeDef *pTim = NULL;
static uint8_t deviceAddr = 0;

/* === Private function declarations =========================================================== */
static bool writeRegister(uint8_t regAddr, uint8_t value);
static bool readRegister(uint8_t regAddr, uint8_t *data, uint8_t size);
static bool calibrateAccel(void);
static void processAccelData(const uint8_t *buf);

/* === Private function implementation ========================================================= */

/**
 * @brief Write single byte to MPU6050 register (blocking)
 */
static bool writeRegister(uint8_t regAddr, uint8_t value) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(pI2c, deviceAddr << 1, regAddr,
                                                  I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
    return (status == HAL_OK) ? false : true; // Return true on error
}

/**
 * @brief Read bytes from MPU6050 register (blocking)
 */
static bool readRegister(uint8_t regAddr, uint8_t *data, uint8_t size) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(pI2c, deviceAddr << 1, regAddr,
                                                 I2C_MEMADD_SIZE_8BIT, data, size, 100);
    return (status == HAL_OK) ? false : true; // Return true on error
}

/**
 * @brief Calibrate accelerometer (assumes device stationary and level)
 */
static bool calibrateAccel(void) {
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    uint8_t buf[6];

    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        if (readRegister(REG_ACCEL_XOUT_H, buf, 6)) {
            return true; // Error
        }

        int16_t x = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t y = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t z = (int16_t)((buf[4] << 8) | buf[5]);

        sum_x += x;
        sum_y += y;
        sum_z += z;

        HAL_Delay(MIN_MEASURE_TIME);
    }

    state.calX = sum_x / CALIBRATION_SAMPLES;
    state.calY = sum_y / CALIBRATION_SAMPLES;
    state.calZ = (sum_z / CALIBRATION_SAMPLES) - _1G;

    return false; // Success
}

/**
 * @brief Apply EMA filter and deadband to raw accelerometer data
 */
static void processAccelData(const uint8_t *buf) {
    // Extract raw 16-bit values (big-endian) and apply calibration
    int16_t rawX = (int16_t)((buf[0] << 8) | buf[1]) - state.calX;
    int16_t rawY = (int16_t)((buf[2] << 8) | buf[3]) - state.calY;

    // EMA filter: y = y + alpha*(x - y)
    int32_t errX = (int32_t)rawX - state.emaX;
    state.emaX += ((int32_t)ALPHA_Q15 * errX) >> 15;

    int32_t errY = (int32_t)rawY - state.emaY;
    state.emaY += ((int32_t)ALPHA_Q15 * errY) >> 15;

    // Apply deadband around center
    int16_t outX = (int16_t)state.emaX;
    int16_t outY = (int16_t)state.emaY;

    if (outX > -DEADBAND && outX < DEADBAND) outX = 0;
    if (outY > -DEADBAND && outY < DEADBAND) outY = 0;

    // Publish (atomic on 32-bit ARM for aligned 16-bit writes)
    published.tiltX = outX;
    published.tiltY = outY;
}

/* === Public function implementation ========================================================== */

accelStatus_t accelDrvInit(I2C_HandleTypeDef *i2cHandle,
                           TIM_HandleTypeDef *timHandle,
                           uint8_t devAddress,
                           accelPowerMode_t mode) {
    if (!i2cHandle || !timHandle || devAddress == 0) {
        return ACCEL_ERROR;
    }

    pI2c = i2cHandle;
    pTim = timHandle;
    deviceAddr = devAddress;

    bool status = false;

    // Verify device ID
    uint8_t check;
    status |= readRegister(REG_WHO_AM_I, &check, 1);
    if (status || (check != DEV_ID)) {
        return ACCEL_ERROR;
    }

    // Configure power management based on mode
    uint8_t pwrMgmt;

    switch (mode) {
        case ACCEL_LOW_POWER_MODE:
            pwrMgmt = PWR_MGMT | TEMP_DIS | CYCLE | CLKSEL_INTERNAL;
            status |= writeRegister(REG_PWR_MGMT_1, pwrMgmt);

            pwrMgmt = PWR_MGMT | GYRO_OFF | LP_WAKE_CTRL;
            status |= writeRegister(REG_PWR_MGMT_2, pwrMgmt);

            pwrMgmt = PWR_MGMT;
            status |= writeRegister(REG_LPA_ODR, pwrMgmt);
            break;

        case ACCEL_LOW_NOISE_MODE:
            pwrMgmt = PWR_MGMT | TEMP_DIS | CLKSEL_INTERNAL;
            status |= writeRegister(REG_PWR_MGMT_1, pwrMgmt);

            pwrMgmt = PWR_MGMT | GYRO_OFF;
            status |= writeRegister(REG_PWR_MGMT_2, pwrMgmt);

            pwrMgmt = PWR_MGMT;
            status |= writeRegister(REG_ACCEL_CONFIG, pwrMgmt);

            pwrMgmt = PWR_MGMT | ACC_DLPF;
            status |= writeRegister(REG_ACCEL_CONFIG_2, pwrMgmt);
            break;

        default:
            return ACCEL_ERROR;
    }

    if (status) {
        return ACCEL_ERROR;
    }

    // Calibrate accelerometer
    if (calibrateAccel()) {
        return ACCEL_ERROR;
    }

    // Initialize filter state
    state.emaX = 0;
    state.emaY = 0;
    published.tiltX = 0;
    published.tiltY = 0;
    dmaBuffer.busy = 0;

    // Start timer to trigger periodic reads
    if (HAL_TIM_Base_Start_IT(timHandle) != HAL_OK) {
        return ACCEL_ERROR;
    }

    return ACCEL_OK;
}

void accelDrvGetTilt(accelTilt_t *tilt) {
    if (tilt) {
        // Read is atomic on 32-bit ARM for aligned 16-bit access
        tilt->tiltX = published.tiltX;
        tilt->tiltY = published.tiltY;
    }
}

/* === HAL Callbacks (call from main.c or stm32f4xx_it.c) ===================================== */

/**
 * @brief Timer callback - triggers I2C DMA read with watchdog
 *
 * Add this to HAL_TIM_PeriodElapsedCallback() in your project
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == pTim && pI2c != NULL) {
        // Watchdog: if DMA busy for too long, force reset
        if (dmaBuffer.busy) {
            dmaBuffer.busyTicks++;
            if (dmaBuffer.busyTicks >= DMA_TIMEOUT_TICKS) {
                // Timeout - abort and reset
                HAL_I2C_Master_Abort_IT(pI2c, deviceAddr << 1);
                dmaBuffer.busy = 0;
                dmaBuffer.busyTicks = 0;
            }
            return; // Don't start new transfer while busy
        }

        // Start new DMA read
        dmaBuffer.busy = 1;
        dmaBuffer.busyTicks = 0;
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read_DMA(pI2c,
                            deviceAddr << 1,
                            REG_ACCEL_XOUT_H,
                            I2C_MEMADD_SIZE_8BIT,
                            dmaBuffer.buff,
                            6);

        // If DMA start failed, reset busy flag
        if (status != HAL_OK) {
            dmaBuffer.busy = 0;
            dmaBuffer.busyTicks = 0;
        }
    }
}

/**
 * @brief I2C DMA complete callback - processes new data
 *
 * Add this to HAL_I2C_MemRxCpltCallback() in your project
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == pI2c && dmaBuffer.busy) {
        processAccelData(dmaBuffer.buff);
        dmaBuffer.busy = 0;
        dmaBuffer.busyTicks = 0;
    }
}

/**
 * @brief I2C error callback - allows retry
 *
 * Add this to HAL_I2C_ErrorCallback() in your project
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c == pI2c) {
        dmaBuffer.busy = 0; // Allow retry on next timer tick
        dmaBuffer.busyTicks = 0;
    }
}

/* === End of source file ====================================================================== */
