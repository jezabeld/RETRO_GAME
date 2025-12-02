/**
 * @file InputDrv.h
 * @brief Input driver for buttons and analog joystick with debouncing and directional events
 *
 * This driver manages 4 digital buttons with debounce filtering and an analog joystick
 * with EMA filtering and directional state machine. The joystick generates navigation
 * events (UP/DOWN/LEFT/RIGHT) with auto-repeat functionality for menu navigation.
 * ADC data is processed at 100Hz with DMA, and directional events are generated at 50Hz.
 *
 * @date Aug 12, 2025
 * @author jez
 */

#ifndef DRIVERS_INPUTDRV_H_
#define DRIVERS_INPUTDRV_H_

/* === Headers files inclusions ================================================================ */
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

/* === Public macros definitions =============================================================== */

/* === Public data type declarations =========================================================== */
typedef struct {
    int16_t jyX; ///< X axis, normalized to -1024..+1023 (center = 0, 2048 values)
    int16_t jyY; ///< Y axis, normalized to -1024..+1023 (center = 0, 2048 values)
} joystick_t;

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

/**
 * @brief Initialize the input driver for buttons and joystick
 *
 * Configures ADC with DMA for joystick reading at 50Hz, sets up button debounce timers,
 * and initializes joystick directional state machine for navigation events.
 *
 * @param adcHandle Pointer to configured ADC handle for joystick channels
 * @param dmaHandle Pointer to configured DMA handle for circular buffer
 * @param timHandle Pointer to configured timer handle for ADC triggering
 * @return 0 on success, 1 on error (invalid parameters or HAL initialization failure)
 */
uint8_t inputInit(ADC_HandleTypeDef *adcHandle, DMA_HandleTypeDef *dmaHandle, TIM_HandleTypeDef *timHandle);

/**
 * @brief Get current filtered joystick axis values (normalized)
 *
 * Returns the latest EMA-filtered and normalized joystick position values.
 * Values are updated at 50Hz by the DMA complete callback.
 *
 * @param axis Axis to read (0=X axis, 1=Y axis, 2=both axes)
 * @param joyPtr Pointer to joystick_t structure to store the result
 * @return 0 on success, 1 on error (invalid parameters)
 *
 * @note Values normalized: -1024..+1023 range (2048 values), center position = 0
 * @note Hardware-independent output, suitable for direct use in game logic
 */
uint8_t inputGetJoyAxis(uint8_t axis, joystick_t *joyPtr);

/**
 * @brief GPIO EXTI callback for button press detection
 *
 * Handles button press interrupts by starting debounce timers and disabling
 * the corresponding IRQ until debounce is complete. Called automatically by HAL.
 *
 * @param GPIO_Pin Pin that triggered the interrupt
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/**
 * @brief Button A debounce timer callback
 *
 * Verifies button A is still pressed after debounce period and sends INP_BTN_A event.
 * Re-enables IRQ and resets button state. Called automatically by FreeRTOS timer.
 *
 * @param hTimer Timer handle (not used)
 */
void btnAtimerCallback(TimerHandle_t hTimer);

/**
 * @brief Button B debounce timer callback
 *
 * Verifies button B is still pressed after debounce period and sends INP_BTN_B event.
 * Re-enables IRQ and resets button state. Called automatically by FreeRTOS timer.
 *
 * @param hTimer Timer handle (not used)
 */
void btnBtimerCallback(TimerHandle_t hTimer);

/**
 * @brief Button C debounce timer callback
 *
 * Verifies button C is still pressed after debounce period and sends INP_BTN_C event.
 * Re-enables IRQ and resets button state. Called automatically by FreeRTOS timer.
 *
 * @param hTimer Timer handle (not used)
 */
void btnCtimerCallback(TimerHandle_t hTimer);

/**
 * @brief Button D debounce timer callback
 *
 * Verifies button D is still pressed after debounce period and sends INP_BTN_D event.
 * Re-enables IRQ and resets button state. Called automatically by FreeRTOS timer.
 *
 * @param hTimer Timer handle (not used)
 */
void btnDtimerCallback(TimerHandle_t hTimer);

/**
 * @brief ADC DMA half complete callback
 *
 * Processes ADC data from the first half of the DMA buffer, applies EMA filtering,
 * and updates the published joystick position variables. This callback runs at 100Hz
 * but does not generate directional events, only updates position data for reading.
 *
 * @param hadc ADC handle
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc);

/**
 * @brief ADC DMA complete callback
 *
 * Processes ADC data from the second half of the DMA buffer, applies EMA filtering,
 * updates the published joystick position variables, and executes the directional
 * state machine to generate joystick navigation events at 50Hz. This callback handles
 * both position updates and directional event generation.
 *
 * @param hadc ADC handle
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

/* === End of header =========================================================================== */
#endif /* DRIVERS_INPUTDRV_H_ */
