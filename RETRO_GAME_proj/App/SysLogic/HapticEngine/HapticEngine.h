/**
 * @file HapticEngine.h
 * @brief Haptic feedback engine for managing vibration patterns
 *
 * This engine provides high-level haptic feedback control, managing different
 * vibration patterns and effects using the low-level HapticDrv driver.
 * Similar to AudioPlayer relationship with AudioDrv.
 *
 * @date Sep 25, 2025
 * @author jez
 */

#ifndef SYSLOGIC_HAPTICENGINE_H_
#define SYSLOGIC_HAPTICENGINE_H_

#include <stdint.h>
#include "HapticDrv.h"

/* === Public data type declarations =========================================================== */

/**
 * @brief Haptic pattern constants for pattern table
 */
#define HP_LIGHT_CLICK      0   ///< Light single click
#define HP_MEDIUM_CLICK     1   ///< Medium strength click
#define HP_STRONG_CLICK     2   ///< Strong single click
#define HP_DOUBLE_CLICK     3   ///< Double click pattern
#define HP_TRIPLE_CLICK     4   ///< Triple click pattern
#define HP_SOFT_BUZZ        5   ///< Soft continuous buzz
#define HP_ALERT_BUZZ       6   ///< Alert buzz pattern
#define HP_SUCCESS_PULSE    7   ///< Success confirmation pulse
#define HP_ERROR_BUZZ       8   ///< Error indication buzz

/* === Public function declarations ============================================================ */

/**
 * @brief Play a predefined haptic pattern
 *
 * Triggers playback of a predefined haptic feedback pattern.
 * Non-blocking operation - pattern plays asynchronously.
 *
 * @param haptic Pointer to initialized haptic driver instance
 * @param pattern Pattern type to play (use constant declaration from avobe)
 */
void vibrationPlay(haptic_t *haptic, uint8_t pattern);

/**
 * @brief FreeRTOS task for haptic event processing
 *
 * Main haptic engine task that processes haptic events from the event queue.
 * Implemented in HapticEngineTask.c
 *
 * @param pvParameters FreeRTOS task parameters (unused)
 */
void HapticEngineTask(void *pvParameters);

/* === End of header =========================================================================== */
#endif /* SYSLOGIC_HAPTICENGINE_H_ */