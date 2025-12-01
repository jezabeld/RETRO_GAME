/**
 * @file TimerDrv.c
 * @brief Timing utilities implementation
 *
 * @date Nov 29, 2025
 * @author jez
 */

#include "TimerDrv.h"
#include "stm32f4xx_hal.h"

void timerDelayMs(uint32_t ms) {
    // Calibrated for 84MHz STM32F446RE
    // Each iteration takes approximately 4 CPU cycles
    // 84MHz / 4 = 21M iterations per second
    // 21000 iterations ≈ 1ms
    volatile uint32_t count = ms * 21000;
    while (count--) {
        __NOP(); // Prevent compiler optimization
    }
}

