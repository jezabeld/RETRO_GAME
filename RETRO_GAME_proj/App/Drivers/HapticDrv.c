/**
 * @file HapticDrv.c
 * @brief DRV2605 Haptic Motor Driver implementation for STM32F446xx
 *
 * This driver provides low-level control for DRV2605 haptic feedback devices
 * via I2C communication. Supports ERM motor configuration, waveform sequencing,
 * and real-time haptic effect playback.
 *
 * @date Sep 23, 2025
 * @author jez
 */

/* === Headers files inclusions ================================================================ */

#include "HapticDrv.h"
#include "TimerDrv.h"

/* === Private macros definitions ============================================================== */

#ifndef DRV2605_I2C_TIMEOUT_MS
#define DRV2605_I2C_TIMEOUT_MS  10        ///< I2C communication timeout in milliseconds
#endif

#define DRV2605_ADDR (0x5Au << 1) ///< Device I2C address (7-bit address 0x5A shifted left)

#define DRV2605_REG_STATUS 0x00       ///< Status register
#define DRV2605_REG_MODE 0x01         ///< Mode register
/* MODE register values (from Adafruit)
 *    0: Internal trigger, call go() to start playback
 *    1: External trigger, rising edge on IN pin starts playback
 *    2: External trigger, playback follows the state of IN pin
 *    3: PWM/analog input
 *    4: Audio
 *    5: Real-time playback
 *    6: Diagnostics
 *    7: Auto calibration
 */
#define DRV2605_MODE_INTTRIG 0x00     ///< Internal trigger mode
#define DRV2605_MODE_EXTTRIGEDGE 0x01 ///< External edge trigger mode
#define DRV2605_MODE_EXTTRIGLVL 0x02  ///< External level trigger mode
#define DRV2605_MODE_PWMANALOG 0x03   ///< PWM/Analog input mode
#define DRV2605_MODE_AUDIOVIBE 0x04   ///< Audio-to-vibe mode
#define DRV2605_MODE_REALTIME 0x05    ///< Real-time playback (RTP) mode
#define DRV2605_MODE_DIAGNOS 0x06     ///< Diagnostics mode
#define DRV2605_MODE_AUTOCAL 0x07     ///< Auto calibration mode

#define DRV2605_REG_RTPIN 0x02    ///< Real-time playback input register
#define DRV2605_REG_LIBRARY 0x03  ///< Waveform library selection register
#define DRV2605_REG_WAVESEQ1 0x04 ///< Waveform sequence register 1
#define DRV2605_REG_WAVESEQ2 0x05 ///< Waveform sequence register 2
#define DRV2605_REG_WAVESEQ3 0x06 ///< Waveform sequence register 3
#define DRV2605_REG_WAVESEQ4 0x07 ///< Waveform sequence register 4
#define DRV2605_REG_WAVESEQ5 0x08 ///< Waveform sequence register 5
#define DRV2605_REG_WAVESEQ6 0x09 ///< Waveform sequence register 6
#define DRV2605_REG_WAVESEQ7 0x0A ///< Waveform sequence register 7
#define DRV2605_REG_WAVESEQ8 0x0B ///< Waveform sequence register 8

#define DRV2605_REG_GO 0x0C          ///< Go register
#define DRV2605_REG_OVERDRIVE 0x0D   ///< Overdrive time offset register
#define DRV2605_REG_SUSTAINPOS 0x0E  ///< Sustain time offset, positive register
#define DRV2605_REG_SUSTAINNEG 0x0F  ///< Sustain time offset, negative register
#define DRV2605_REG_BREAK 0x10       ///< Brake time offset register
#define DRV2605_REG_AUDIOCTRL 0x11   ///< Audio-to-vibe control register
#define DRV2605_REG_AUDIOLVL 0x12    ///< Audio-to-vibe minimum input level register
#define DRV2605_REG_AUDIOMAX 0x13    ///< Audio-to-vibe maximum input level register
#define DRV2605_REG_AUDIOOUTMIN 0x14 ///< Audio-to-vibe minimum output drive register
#define DRV2605_REG_AUDIOOUTMAX 0x15 ///< Audio-to-vibe maximum output drive register
#define DRV2605_REG_RATEDV 0x16      ///< Rated voltage register
#define DRV2605_REG_CLAMPV 0x17      ///< Overdrive clamp voltage register
#define DRV2605_REG_AUTOCALCOMP 0x18 ///< Auto-calibration compensation result register
#define DRV2605_REG_AUTOCALEMP 0x19  ///< Auto-calibration back-EMF result register
#define DRV2605_REG_FEEDBACK 0x1A    ///< Feedback control register
#define DRV2605_REG_CONTROL1 0x1B    ///< Control1 Register
#define DRV2605_REG_CONTROL2 0x1C    ///< Control2 Register
#define DRV2605_REG_CONTROL3 0x1D    ///< Control3 Register
#define DRV2605_REG_CONTROL4 0x1E    ///< Control4 Register
#define DRV2605_REG_VBAT 0x21        ///< Vbat voltage-monitor register
#define DRV2605_REG_LRARESON 0x22    ///< LRA resonance-period register

/* === Private function declarations =========================================================== */

static void hapticDelay(uint32_t ms);
static HAL_StatusTypeDef _wr(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val);
static HAL_StatusTypeDef _rd(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *val);
static HAL_StatusTypeDef _soft_reset(I2C_HandleTypeDef *hi2c);

/* === Private function implementation ========================================================= */

/**
 * @brief Write single byte to DRV2605 register
 * @param hi2c Pointer to I2C handle
 * @param reg Register address
 * @param val Value to write
 * @return HAL_StatusTypeDef Operation status
 */
static HAL_StatusTypeDef _wr(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val) {
    return HAL_I2C_Mem_Write(hi2c, DRV2605_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             &val, 1, DRV2605_I2C_TIMEOUT_MS);
}

/**
 * @brief Read single byte from DRV2605 register
 * @param hi2c Pointer to I2C handle
 * @param reg Register address
 * @param val Pointer to store read value
 * @return HAL_StatusTypeDef Operation status
 */
static HAL_StatusTypeDef _rd(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *val) {
    return HAL_I2C_Mem_Read(hi2c, DRV2605_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                            val, 1, DRV2605_I2C_TIMEOUT_MS);
}

/**
 * @brief Perform soft reset of DRV2605
 * @param hi2c Pointer to I2C handle
 * @return HAL_StatusTypeDef Operation status
 */
static HAL_StatusTypeDef _soft_reset(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_MODE, 0x80);
    timerDelayMs(2);
    return st;
}

/* === Public function implementation ========================================================== */

// uint8_t hapticSetIntensity(I2C_HandleTypeDef *hi2c, uint8_t rated, uint8_t clamp) {
//     HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_RATEDV, rated);
//     if (st) return (st != HAL_OK);
//     return (_wr(hi2c, DRV2605_REG_CLAMPV, clamp) != HAL_OK);
// }

uint8_t hapticInit(haptic_t *haptic, I2C_HandleTypeDef *hi2c) {
    if (!haptic || !hi2c) {
        return 1;
    }

    haptic->hi2c = hi2c;
    HAL_StatusTypeDef st;

    // Reset and set to INTTRIG mode
    st = _soft_reset(hi2c);                                  if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_MODE, DRV2605_MODE_INTTRIG);  if (st) return 1;

    // Disable real-time-playback
    st = _wr(hi2c, DRV2605_REG_RTPIN, 0x00); 	            if (st) return 1;

    // Set strong click + end sequence
    st = _wr(hi2c, DRV2605_REG_WAVESEQ1, 1); 	            if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_WAVESEQ2, 0); 	            if (st) return 1;

    // Disable overdrive
    st = _wr(hi2c, DRV2605_REG_OVERDRIVE, 0); 	            if (st) return 1;

    st = _wr(hi2c, DRV2605_REG_SUSTAINPOS, 0); 	            if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_SUSTAINNEG, 0); 	            if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_BREAK, 0); 	                if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_AUDIOMAX, 0x64); 	        if (st) return 1;

    // ERM: FEEDBACK bit7=0 (ERM). 0x36 is safe baseline (gain/BEMF)
    st = _wr(hi2c, DRV2605_REG_FEEDBACK, 0x36);              if (st) return 1;

    // CONTROL3 = 0x20 → ERM open-loop
    st = _wr(hi2c, DRV2605_REG_CONTROL3, 0x20);              if (st) return 1;

    return (st != HAL_OK);
}

uint8_t hapticSelectLibrary(haptic_t *haptic, uint8_t lib){
    if (!haptic || !haptic->hi2c) {
        return 1;
    }
    return (_wr(haptic->hi2c, DRV2605_REG_LIBRARY, lib) != HAL_OK);
}

uint8_t hapticGoSequence(haptic_t *haptic){
    if (!haptic || !haptic->hi2c) {
        return 1;
    }
    return (_wr(haptic->hi2c, DRV2605_REG_GO, 1) != HAL_OK);
}

uint8_t hapticStopSequence(haptic_t *haptic){
    if (!haptic || !haptic->hi2c) {
        return 1;
    }
    return (_wr(haptic->hi2c, DRV2605_REG_GO, 0) != HAL_OK);
}

uint8_t hapticSetWaveform(haptic_t *haptic, uint8_t slot, uint8_t w){
    if (!haptic || !haptic->hi2c || slot > 7) {
        return 1;
    }
    return (_wr(haptic->hi2c, DRV2605_REG_WAVESEQ1 + slot, w) != HAL_OK);
}

uint8_t hapticSetSequence(haptic_t *haptic, uint8_t *steps, uint8_t n){
    if (!haptic || !haptic->hi2c || !steps || n > 8) {
        return 1;
    }

    uint8_t st = 0;
    for (int i = 0; i < 8; i++) {
        if (i < n) {
            st += hapticSetWaveform(haptic, i, steps[i]);
        } else {
            st += hapticSetWaveform(haptic, i, 0);
        }
    }
    return (st > 0);
}

uint8_t hapticTrigger(haptic_t *haptic, uint8_t id) {
    if (!haptic || !haptic->hi2c) {
        return 1;
    }

	uint8_t st = 0;
	st += hapticSetWaveform(haptic, 0, id);
	st += hapticGoSequence(haptic);
	return (st > 0);
}

/* === End of source file ====================================================================== */
