/**
 * @file TimerDrv.h
 * @brief Timing utilities for embedded system
 *
 * Provides timing functions including pre-RTOS busy-wait delays.
 *
 * @date Nov 29, 2025
 * @author jez
 */

#ifndef DRIVERS_TIMERDRV_H_
#define DRIVERS_TIMERDRV_H_

#include <stdint.h>

/**
 * @brief Busy-wait delay for pre-RTOS initialization
 *
 * CPU cycle-based delay that works before FreeRTOS scheduler starts.
 * This function blocks the CPU in a tight loop and should only be used
 * during hardware initialization before the scheduler is running.
 *
 * After the scheduler starts, use vTaskDelay() or vTaskDelayUntil() instead.
 *
 * @param ms Delay time in milliseconds
 *
 * @note Calibrated for 84MHz STM32F446RE CPU
 * @note Do NOT use after FreeRTOS scheduler has started
 * @note Accuracy depends on CPU frequency and compiler optimization
 */
void timerDelayMs(uint32_t ms);

#endif /* DRIVERS_TIMERDRV_H_ */
