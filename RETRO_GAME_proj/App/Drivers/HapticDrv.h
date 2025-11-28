/**
 * @file HapticDrv.h
 * @brief DRV2605 Haptic Motor Driver for STM32F446xx
 *
 * This driver provides an interface to control DRV2605-based haptic feedback devices
 * using I2C communication. It supports waveform library selection, sequence programming,
 * and real-time haptic effect playback.
 *
 * @date Sep 23, 2025
 * @author jez
 *
 * @note This driver is designed for ERM (Eccentric Rotating Mass) motors by default
 */

#ifndef DRIVERS_HAPTICDRV_H_
#define DRIVERS_HAPTICDRV_H_

/* === Headers files inclusions ================================================================ */

#include <stdint.h>
#include "stm32f4xx_hal.h"

/* === Public data type declarations =========================================================== */

/**
 * @brief Haptic device handle structure
 *
 * This structure contains the I2C interface information needed to communicate
 * with the DRV2605 haptic feedback controller.
 */
typedef struct {
    I2C_HandleTypeDef *hi2c; ///< Pointer to I2C handle for device communication
} haptic_t;

/* === Public function declarations ============================================================ */

/**
 * @brief Initialize the haptic feedback device
 *
 * Initializes the DRV2605 haptic controller with default ERM motor settings.
 * Performs soft reset and configures basic operation parameters.
 *
 * @param haptic Pointer to haptic handle structure
 * @param hi2c Pointer to I2C handle for communication
 * @return 0 on success, 1 on error (communication failure or invalid parameters)
 */
uint8_t hapticInit(haptic_t *haptic, I2C_HandleTypeDef *hi2c);

/**
 * @brief Select haptic waveform library
 *
 * Selects the waveform library to use for effect playback.
 *
 * @param haptic Pointer to haptic handle
 * @param lib Library selection (0=empty, 1-5=ERM libraries, 6=LRA library)
 * @return 0 on success, 1 on error (communication failure)
 */
uint8_t hapticSelectLibrary(haptic_t *haptic, uint8_t lib);

/**
 * @brief Start haptic sequence playback
 *
 * Triggers playback of the programmed waveform sequence.
 *
 * @param haptic Pointer to haptic handle
 * @return 0 on success, 1 on error (communication failure)
 */
uint8_t hapticGoSequence(haptic_t *haptic);

/**
 * @brief Stop haptic sequence playback
 *
 * Immediately stops any active haptic sequence.
 *
 * @param haptic Pointer to haptic handle
 * @return 0 on success, 1 on error (communication failure)
 */
uint8_t hapticStopSequence(haptic_t *haptic);

/**
 * @brief Assign a waveform to a sequencer slot
 *
 * Programs a specific waveform effect into one of the 8 sequencer slots.
 * See DRV2605 datasheet section 11.2 for complete list of effects.
 *
 * @param haptic Pointer to haptic handle
 * @param slot Sequencer slot (0-7)
 * @param w Waveform effect ID (1-123, 0=end sequence)
 * @return 0 on success, 1 on error (communication failure)
 * @note Reference: https://cdn-shop.adafruit.com/datasheets/DRV2605.pdf
 */
uint8_t hapticSetWaveform(haptic_t *haptic, uint8_t slot, uint8_t w);

/**
 * @brief Set complete haptic sequence
 *
 * Programs up to 8 waveform steps into the sequencer. Unused slots are cleared.
 *
 * @param haptic Pointer to haptic handle
 * @param steps Pointer to array of effect IDs
 * @param n Number of steps in array (1-8)
 * @return 0 on success, 1 on error (invalid parameters or communication failure)
 */
uint8_t hapticSetSequence(haptic_t *haptic, uint8_t *steps, uint8_t n);

/**
 * @brief Trigger single haptic effect
 *
 * Programs a single effect into slot 0 and immediately starts playback.
 * Convenience function for simple single-effect haptic feedback.
 *
 * @param haptic Pointer to haptic handle
 * @param id Effect ID (1-123)
 * @return 0 on success, 1 on error(communication failure)
 */
uint8_t hapticTrigger(haptic_t *haptic, uint8_t id);


#endif /* DRIVERS_HAPTICDRV_H_ */