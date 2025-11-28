/**
 * @file HapticEngine.c
 * @brief Haptic feedback engine implementation
 *
 * Provides high-level haptic feedback control with pattern management,
 * timing control, and intensity scaling. Acts as middleware between
 * system events and the low-level HapticDrv.
 *
 * @date Sep 25, 2025
 * @author jez
 */

/* === Headers files inclusions ================================================================ */

#include "HapticEngine.h"
#include "cmsis_os.h"
#include "synchronization.h"

/* === Private macros definitions ============================================================== */

/* === External variable declarations ========================================================== */
extern haptic_t hapticDevice;

/* === Private data type declarations ========================================================== */

/**
 * @brief Haptic pattern definition
 */
typedef struct {
    const uint8_t* steps; ///< Pointer to pattern steps array
    uint8_t step_count;   ///< Number of steps in pattern
} hapticPatternDef_t;

/* === Private variable declarations =========================================================== */

// Pattern lookup table
static const hapticPatternDef_t pattern_table[] = {
    {(uint8_t[]){14}, 1},           // HP_LIGHT_CLICK
    {(uint8_t[]){1}, 1},            // HP_MEDIUM_CLICK
    {(uint8_t[]){47}, 1},           // HP_STRONG_CLICK
    {(uint8_t[]){14, 14}, 2},       // HP_DOUBLE_CLICK
    {(uint8_t[]){14, 14, 14}, 3},   // HP_TRIPLE_CLICK
    {(uint8_t[]){17}, 1},           // HP_SOFT_BUZZ
    {(uint8_t[]){10, 10, 10}, 3},   // HP_ALERT_BUZZ
    {(uint8_t[]){15, 16}, 2},       // HP_SUCCESS_PULSE
    {(uint8_t[]){58, 58, 58}, 3}    // HP_ERROR_BUZZ
};

/* === Private function declarations =========================================================== */

/* === Private function implementation ========================================================= */

/* === Public function implementation ========================================================== */

void vibrationPlay(haptic_t *haptic, uint8_t pattern) {
    if (!haptic || pattern >= (sizeof(pattern_table) / sizeof(pattern_table[0]))) {
        return;
    }

    const hapticPatternDef_t* pattern_def = &pattern_table[pattern];

    if (pattern_def->steps && pattern_def->step_count > 0) {
        // Use HapticDrv directly to set and play the sequence
        hapticSetSequence(haptic, (uint8_t*)pattern_def->steps, pattern_def->step_count);
        hapticGoSequence(haptic);
    }
}

void HapticEngineTask(void *pvParameters) {
    (void)pvParameters;

    event_id_t receivedEvent;

    // Main task loop
    for (;;) {
        // Wait for haptic events with timeout
        if (xQueueReceive(qHaptic, &receivedEvent, portMAX_DELAY) == pdTRUE) {

            switch (receivedEvent) {
                case HAP_LIGHT_CLICK:
                    vibrationPlay(&hapticDevice, HP_LIGHT_CLICK);
                    break;

                case HAP_MEDIUM_CLICK:
                    vibrationPlay(&hapticDevice, HP_MEDIUM_CLICK);
                    break;

                case HAP_STRONG_CLICK:
                    vibrationPlay(&hapticDevice, HP_STRONG_CLICK);
                    break;

                case HAP_DOUBLE_CLICK:
                    vibrationPlay(&hapticDevice, HP_DOUBLE_CLICK);
                    break;

                case HAP_TRIPLE_CLICK:
                    vibrationPlay(&hapticDevice, HP_TRIPLE_CLICK);
                    break;

                case HAP_SOFT_BUZZ:
                    vibrationPlay(&hapticDevice, HP_SOFT_BUZZ);
                    break;

                case HAP_ALERT_BUZZ:
                    vibrationPlay(&hapticDevice, HP_ALERT_BUZZ);
                    break;

                case HAP_SUCCESS_PULSE:
                    vibrationPlay(&hapticDevice, HP_SUCCESS_PULSE);
                    break;

                case HAP_ERROR_BUZZ:
                    vibrationPlay(&hapticDevice, HP_ERROR_BUZZ);
                    break;

                default:
                    // Event not handled
                    break;
            }
        }
    }
}

/* === End of source file ====================================================================== */