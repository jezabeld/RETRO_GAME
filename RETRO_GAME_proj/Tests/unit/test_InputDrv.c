/**
 * @file test_InputDrv.c
 *
 * @date Sep 19, 2025
 * @author jez
 */

/* === Headers files inclusions ================================================================ */

#include "unity.h"
#include "mock_cmsis_os.h"
#include "mock_stm32f446xx.h"
#include "mock_stm32f4xx_hal.h"
#include "mock_stm32f4xx_hal_gpio.h"
#include "mock_stm32f4xx_hal_dma.h"
#include "mock_stm32f4xx_hal_adc.h"
#include "mock_stm32f4xx_hal_tim.h"
#include <string.h>

#include "InputDrv.h"
#include "synchronization.h"

/* === External access to DMA buffer for testing =============================================== */
typedef struct {
    uint16_t buff[40];
    uint32_t fullLen;
    uint32_t halfLen;
} dmaBuffer_t;

extern dmaBuffer_t dmaBuffer;

/* === Test helper variables and functions ==================================================== */
static event_id_t captured_event;
static int event_capture_enabled = 0;

// Callback to capture the event value sent to the queue
BaseType_t capture_queue_event(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait,
                               int cmock_num_calls) {
    if (event_capture_enabled && pvItemToQueue != NULL) {
        captured_event = *(event_id_t *) pvItemToQueue;
    }
    return pdTRUE;
}

// Callback for ISR version
BaseType_t capture_queue_event_from_isr(QueueHandle_t xQueue, const void *pvItemToQueue,
                                        BaseType_t *pxHigherPriorityTaskWoken, int cmock_num_calls) {
    if (event_capture_enabled && pvItemToQueue != NULL) {
        captured_event = *(event_id_t *) pvItemToQueue;
    }
    return pdTRUE;
}

/* === Private variables ======================================================================= */
static ADC_HandleTypeDef mock_adc;
static DMA_HandleTypeDef mock_dma;
static TIM_HandleTypeDef mock_tim;

/* FreeRTOS Timer handles used by InputDrv */
TimerHandle_t tBtnAdebounce = (TimerHandle_t) 0x1000;
TimerHandle_t tBtnBdebounce = (TimerHandle_t) 0x1001;
TimerHandle_t tBtnCdebounce = (TimerHandle_t) 0x1002;
TimerHandle_t tBtnDdebounce = (TimerHandle_t) 0x1003;

/* FreeRTOS Queue handle used by InputDrv */
QueueHandle_t qEvents = (QueueHandle_t) 0x2000;

/* Semaphore handles used by InputDrv */
SemaphoreHandle_t semGFXReady = (SemaphoreHandle_t) 0x3000;
SemaphoreHandle_t semUiReady = (SemaphoreHandle_t) 0x3001;

// Struct to test multiple buttons
typedef struct {
    TimerHandle_t hTimer;
    uint16_t pin;
    void (*timerCb)(TimerHandle_t);
    IRQn_Type irq;
    char *btn;
    uint32_t expectedEvent; // Expected event ID for this button
} buttons_t;

// Array of buttons info
buttons_t buttons[] = { { NULL, GPIO_PIN_1, btnAtimerCallback, EXTI1_IRQn, "btnA", 65 },   // INP_BTN_A = 65
                        { NULL, GPIO_PIN_0, btnBtimerCallback, EXTI0_IRQn, "btnB", 66 },   // INP_BTN_B = 66
                        { NULL, GPIO_PIN_2, btnCtimerCallback, EXTI2_IRQn, "btnC", 67 },   // INP_BTN_C = 67
                        { NULL, GPIO_PIN_3, btnDtimerCallback, EXTI3_IRQn, "btnD", 68 } }; // INP_BTN_D = 68

/* === Private function implementation ========================================================= */
void setUp() {
    // Reset mock handles before each test
    memset(&mock_adc, 0, sizeof(ADC_HandleTypeDef));
    memset(&mock_dma, 0, sizeof(DMA_HandleTypeDef));
    memset(&mock_tim, 0, sizeof(TIM_HandleTypeDef));

    // Initialize button timers in structure
    buttons[0].hTimer = tBtnAdebounce;
    buttons[1].hTimer = tBtnBdebounce;
    buttons[2].hTimer = tBtnCdebounce;
    buttons[3].hTimer = tBtnDdebounce;

    // Initialize driver with mocks for tests that need initialized state
    HAL_ADC_Start_DMA_ExpectAndReturn(&mock_adc, (uint32_t *) 0, 40, HAL_OK);
    HAL_TIM_Base_Start_ExpectAndReturn(&mock_tim, HAL_OK);
    HAL_ADC_Start_DMA_IgnoreArg_pData();
    inputInit(&mock_adc, &mock_dma, &mock_tim);
}

void tearDown() {
    // Clean up event capture after each test
    event_capture_enabled = 0;
    captured_event = 0;
}

/**** inputInit ****/

/**
 * @brief Test inputInit parameter validation
 *
 * Verifies that inputInit returns error code (1) when called with NULL parameters.
 * Tests all combinations of NULL pointers for ADC, DMA, and Timer handles.
 *
 * @test Calls inputInit with various NULL parameter combinations
 * @expected Function returns 1 (error) for all NULL parameter cases
 */
void test_inputInit_invalid_parameters_return_error_code(void) {
    // Test NULL ADC handle
    TEST_ASSERT_EQUAL(1, inputInit(NULL, &mock_dma, &mock_tim));

    // Test NULL DMA handle
    TEST_ASSERT_EQUAL(1, inputInit(&mock_adc, NULL, &mock_tim));

    // Test NULL timer handle
    TEST_ASSERT_EQUAL(1, inputInit(&mock_adc, &mock_dma, NULL));

    // Test all NULL
    TEST_ASSERT_EQUAL(1, inputInit(NULL, NULL, NULL));
}

/**
 * @brief Test inputInit successful initialization
 *
 * Verifies that inputInit correctly initializes the input driver when provided
 * with valid parameters. Mocks HAL_ADC_Start_DMA and HAL_TIM_Base_Start to
 * return success, then verifies the function returns 0.
 *
 * @test Calls inputInit with valid ADC, DMA, and Timer handles
 * @expected Function returns 0 (success) and calls HAL functions with correct parameters
 */
void test_inputInit_valid_parameters_return_success(void) {
    // Setup mocks to expect HAL function calls with correct parameters
    HAL_ADC_Start_DMA_ExpectAndReturn(&mock_adc, (uint32_t *) 0, 40, HAL_OK);
    HAL_TIM_Base_Start_ExpectAndReturn(&mock_tim, HAL_OK);

    // Ignore the buffer pointer since we can't predict its exact address
    HAL_ADC_Start_DMA_IgnoreArg_pData();

    // Test valid parameters - should return success (0)
    TEST_ASSERT_EQUAL(0, inputInit(&mock_adc, &mock_dma, &mock_tim));
}

/**
 * @brief Test inputInit ADC initialization failure handling
 *
 * Verifies that inputInit correctly handles ADC initialization failures.
 * Mocks HAL_ADC_Start_DMA to return HAL_ERROR and verifies the function
 * propagates the error by returning 1.
 *
 * @test Calls inputInit with HAL_ADC_Start_DMA configured to fail
 * @expected Function returns 1 (error) when ADC initialization fails
 */
void test_inputInit_adc_start_dma_fails_return_error(void) {
    // Setup mock to simulate ADC_Start_DMA failure
    HAL_ADC_Start_DMA_ExpectAndReturn(&mock_adc, (uint32_t *) 0, 40, HAL_ERROR);
    HAL_ADC_Start_DMA_IgnoreArg_pData();

    // Test should return error (1) when HAL_ADC_Start_DMA fails
    TEST_ASSERT_EQUAL(1, inputInit(&mock_adc, &mock_dma, &mock_tim));
}

/**
 * @brief Test inputInit Timer initialization failure handling
 *
 * Verifies that inputInit correctly handles Timer initialization failures.
 * Mocks HAL_ADC_Start_DMA to succeed but HAL_TIM_Base_Start to return HAL_ERROR,
 * then verifies the function propagates the error by returning 1.
 *
 * @test Calls inputInit with HAL_TIM_Base_Start configured to fail
 * @expected Function returns 1 (error) when Timer initialization fails
 */
void test_inputInit_tim_base_start_fails_return_error(void) {
    // Setup mocks - ADC succeeds but Timer fails
    HAL_ADC_Start_DMA_ExpectAndReturn(&mock_adc, (uint32_t *) 0, 40, HAL_OK);
    HAL_TIM_Base_Start_ExpectAndReturn(&mock_tim, HAL_ERROR);
    HAL_ADC_Start_DMA_IgnoreArg_pData();

    // Test should return error (1) when HAL_TIM_Base_Start fails
    TEST_ASSERT_EQUAL(1, inputInit(&mock_adc, &mock_dma, &mock_tim));
}

/**** inputGetJoyAxis ****/

/**
 * @brief Test inputGetJoyAxis NULL pointer validation
 *
 * Verifies that inputGetJoyAxis correctly validates the joystick pointer parameter.
 * Tests all valid axis values (0, 1, 2) with NULL joystick pointer.
 *
 * @test Calls inputGetJoyAxis with NULL joystick pointer for each axis
 * @expected Function returns 1 (error) for all NULL pointer cases
 */
void test_inputGetJoyAxis_null_pointer_returns_error(void) {
    // Test NULL joystick pointer
    TEST_ASSERT_EQUAL(1, inputGetJoyAxis(0, NULL));
    TEST_ASSERT_EQUAL(1, inputGetJoyAxis(1, NULL));
    TEST_ASSERT_EQUAL(1, inputGetJoyAxis(2, NULL));
}

/**
 * @brief Test inputGetJoyAxis axis parameter validation
 *
 * Verifies that inputGetJoyAxis correctly validates the axis parameter.
 * Tests invalid axis values (>2) with valid joystick pointer.
 * Valid axis range is 0-2 (X-only, Y-only, Both).
 *
 * @test Calls inputGetJoyAxis with invalid axis values (3, 255)
 * @expected Function returns 1 (error) for invalid axis values
 */
void test_inputGetJoyAxis_invalid_axis_returns_error(void) {
    joystick_t joy;

    // Test invalid axis values (valid range is 0-2)
    TEST_ASSERT_EQUAL(1, inputGetJoyAxis(3, &joy));
    TEST_ASSERT_EQUAL(1, inputGetJoyAxis(255, &joy));
}

/**
 * @brief Test inputGetJoyAxis X-axis only updating
 *
 * Verifies that inputGetJoyAxis correctly updates only the X-axis when axis=0.
 * Uses driver initialized in setUp() with center values (JY_CENTER_X = 2048).
 *
 * @test Calls inputGetJoyAxis with axis=0 (X-only)
 * @expected Function returns 0, jyX=2048, jyY remains unchanged (0)
 */
void test_inputGetJoyAxis_x_axis_returns_success(void) {
    joystick_t joy = { 0 };

    // Test X axis only (axis = 0) - driver initialized in setUp
    TEST_ASSERT_EQUAL(0, inputGetJoyAxis(0, &joy));

    // Verify that jyX was updated with initialized value (JY_CENTER_X = 2048)
    TEST_ASSERT_EQUAL(2048, joy.jyX);
    // jyY should remain unchanged (0) since axis=0 only reads X
    TEST_ASSERT_EQUAL(0, joy.jyY);
}

/**
 * @brief Test inputGetJoyAxis Y-axis only updating
 *
 * Verifies that inputGetJoyAxis correctly updates only the Y-axis when axis=1.
 * Uses driver initialized in setUp() with center values (JY_CENTER_Y = 2048).
 *
 * @test Calls inputGetJoyAxis with axis=1 (Y-only)
 * @expected Function returns 0, jyY=2048, jyX remains unchanged (0)
 */
void test_inputGetJoyAxis_y_axis_returns_success(void) {
    joystick_t joy = { 0 };

    // Test Y axis only (axis = 1) - driver initialized in setUp
    TEST_ASSERT_EQUAL(0, inputGetJoyAxis(1, &joy));

    // Verify that jyY was updated with initialized value (JY_CENTER_Y = 2048)
    TEST_ASSERT_EQUAL(2048, joy.jyY);
    // jyX should remain unchanged (0) since axis=1 only reads Y
    TEST_ASSERT_EQUAL(0, joy.jyX);
}

/**
 * @brief Test inputGetJoyAxis both axes updating
 *
 * Verifies that inputGetJoyAxis correctly updates both X and Y axes when axis=2.
 * Uses driver initialized in setUp() with center values.
 *
 * @test Calls inputGetJoyAxis with axis=2 (both axes)
 * @expected Function returns 0, jyX=2048, jyY=2048
 */
void test_inputGetJoyAxis_both_axes_returns_success(void) {
    joystick_t joy = { 0 };

    // Test both axes (axis = 2) - driver initialized in setUp
    TEST_ASSERT_EQUAL(0, inputGetJoyAxis(2, &joy));

    // Verify that both axes were updated with initialized values
    TEST_ASSERT_EQUAL(2048, joy.jyX);
    TEST_ASSERT_EQUAL(2048, joy.jyY);
}

/**** HAL_GPIO_EXTI_Callback ****/

/**
 * @brief Test HAL_GPIO_EXTI_Callback debounce timer start for all buttons
 *
 * Verifies that HAL_GPIO_EXTI_Callback correctly starts debounce timers and
 * disables interrupts for all four buttons (A, B, C, D). Uses loop-based testing
 * with buttons array to test all buttons with their respective pins, timers, and IRQs.
 *
 * @test Calls HAL_GPIO_EXTI_Callback for each button pin
 * @expected For each button: xTimerStartFromISR called, HAL_NVIC_DisableIRQ called
 */
void test_HAL_GPIO_EXTI_Callback_all_buttons_start_debounce_timers(void) {
    for (int i = 0; i < 4; i++) {
        // Setup mocks for button EXTI callback
        xTimerStartFromISR_ExpectAndReturn(buttons[i].hTimer, NULL, pdTRUE);
        xTimerStartFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
        HAL_NVIC_DisableIRQ_Expect(buttons[i].irq);

        // Trigger EXTI callback for this button
        HAL_GPIO_EXTI_Callback(buttons[i].pin);
        // If we get here without mock failure, the test passed for this button
        TEST_ASSERT_TRUE_MESSAGE(1, buttons[i].btn);
    }
}

/**
 * @brief Test HAL_GPIO_EXTI_Callback with unknown pin
 *
 * Verifies that HAL_GPIO_EXTI_Callback correctly ignores interrupt callbacks
 * for pins not assigned to any button. No HAL functions should be called.
 *
 * @test Calls HAL_GPIO_EXTI_Callback with GPIO_PIN_4 (unused pin)
 * @expected No mock functions called, no side effects
 */
void test_HAL_GPIO_EXTI_Callback_unknown_pin_does_nothing(void) {
    // No calls expected - unknown pin should not trigger any calls

    // Trigger EXTI callback for unknown pin
    HAL_GPIO_EXTI_Callback(GPIO_PIN_4); // Pin not used by any button
}

/**** btn*timerCallback ****/

/**
 * @brief Test button timer callbacks when button is not pressed
 *
 * Verifies that button timer callbacks correctly handle the case when the button
 * is not pressed during debounce timeout. Uses loop-based testing for all buttons.
 * Should only re-enable the IRQ without sending any events to the queue.
 *
 * @test Calls each button timer callback with GPIO_PIN_SET (button not pressed)
 * @expected For each button: HAL_NVIC_EnableIRQ called, no xQueueSend called
 */
void test_button_timer_callbacks_button_not_pressed_only_enables_irq(void) {
    for (int i = 0; i < 4; i++) {
        // Setup: button not pressed (GPIO_PIN_SET for active low buttons)
        HAL_GPIO_ReadPin_ExpectAndReturn(NULL, buttons[i].pin, GPIO_PIN_SET);
        HAL_GPIO_ReadPin_IgnoreArg_GPIOx();

        // Expect IRQ to be re-enabled (always happens)
        HAL_NVIC_EnableIRQ_Expect(buttons[i].irq);

        // No queue send expected since button is not pressed

        // Call the timer callback
        buttons[i].timerCb(NULL);
        // If we get here without mock failure, the test passed for this button
        TEST_ASSERT_TRUE_MESSAGE(1, buttons[i].btn);
    }
}

/**
 * @brief Test button timer callbacks when button is pressed
 *
 * Verifies that button timer callbacks correctly handle the button press sequence.
 * First calls HAL_GPIO_EXTI_Callback to set button state to PRESS_PENDING,
 * then calls timer callback with GPIO_PIN_RESET (button pressed).
 * Should send the correct event ID to queue and re-enable IRQ.
 *
 * @test For each button: EXTI callback → timer callback with button pressed
 * @expected xQueueSend called with correct event ID, HAL_NVIC_EnableIRQ called
 */
void test_button_timer_callbacks_button_pressed_sends_event(void) {
    for (int i = 0; i < 4; i++) {
        // Setup mocks for HAL_GPIO_EXTI_Callback (sets button to PRESS_PENDING)
        xTimerStartFromISR_ExpectAndReturn(buttons[i].hTimer, NULL, pdTRUE);
        xTimerStartFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
        HAL_NVIC_DisableIRQ_Expect(buttons[i].irq);

        // Call EXTI callback to set button state to PRESS_PENDING
        HAL_GPIO_EXTI_Callback(buttons[i].pin);

        // Setup: button pressed (GPIO_PIN_RESET for active low buttons)
        HAL_GPIO_ReadPin_ExpectAndReturn(NULL, buttons[i].pin, GPIO_PIN_RESET);
        HAL_GPIO_ReadPin_IgnoreArg_GPIOx();

        // Enable event capture and expect event to be sent to queue
        event_capture_enabled = 1;
        xQueueSend_AddCallback(capture_queue_event);
        xQueueSend_ExpectAndReturn(qEvents, NULL, 0, pdTRUE);
        xQueueSend_IgnoreArg_pvItemToQueue();

        // Expect IRQ to be re-enabled
        HAL_NVIC_EnableIRQ_Expect(buttons[i].irq);

        // Call the timer callback
        buttons[i].timerCb(NULL);

        // Verify the correct event was captured
        TEST_ASSERT_EQUAL_MESSAGE(buttons[i].expectedEvent, captured_event, buttons[i].btn);
    }
}

/**** HAL_ADC_ConvHalfCpltCallback ****/

/**
 * @brief Test HAL_ADC_ConvHalfCpltCallback with wrong ADC instance
 *
 * Verifies that HAL_ADC_ConvHalfCpltCallback correctly ignores callbacks
 * from ADC instances other than ADC1. Joystick values should remain unchanged.
 *
 * @test Calls HAL_ADC_ConvHalfCpltCallback with mock ADC instance != ADC1
 * @expected No joystick value changes, no side effects
 */
void test_HAL_ADC_ConvHalfCpltCallback_wrong_adc_instance_does_nothing(void) {
    ADC_HandleTypeDef mock_adc_wrong = { 0 };
    joystick_t joy_before, joy_after;

    // Get initial values
    inputGetJoyAxis(2, &joy_before);

    // Set up wrong ADC instance (not ADC1)
    mock_adc_wrong.Instance = (void *) 0x12345; // Any value different from ADC1

    // Call callback with wrong ADC instance - should do nothing
    HAL_ADC_ConvHalfCpltCallback(&mock_adc_wrong);

    // Get values after callback
    inputGetJoyAxis(2, &joy_after);

    // Values should remain unchanged
    TEST_ASSERT_EQUAL(joy_before.jyX, joy_after.jyX);
    TEST_ASSERT_EQUAL(joy_before.jyY, joy_after.jyY);
}

/**
 * @brief Test HAL_ADC_ConvHalfCpltCallback processing first half of DMA buffer
 *
 * Verifies that HAL_ADC_ConvHalfCpltCallback correctly processes the first half
 * of the DMA buffer (elements 0-19) using EMA filtering. Injects known values
 * into buffer and verifies exact EMA calculation results.
 * EMA formula: y = y + alpha*(x - y), ALPHA_Q15 = 23675 (≈0.72)
 *
 * @test Calls callback with ADC1, DMA buffer filled with X=3000, Y=1500
 * @expected Joystick values updated via EMA: jyX=2735, jyY=1653
 */
void test_HAL_ADC_ConvHalfCpltCallback_processes_first_half_buffer(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    joystick_t joy_before, joy_after;

    // Get initial values (center values from setUp)
    inputGetJoyAxis(2, &joy_before);

    // Inject specific values into the first half of DMA buffer (first 20 elements)
    // Buffer contains interleaved X,Y samples: X0,Y0,X1,Y1,X2,Y2...
    // Set all X values to 3000 and Y values to 1500
    for (int i = 0; i < 20; i += 2) {
        dmaBuffer.buff[i] = 3000;     // X samples
        dmaBuffer.buff[i + 1] = 1500; // Y samples
    }

    // Set up correct ADC instance
    mock_adc_correct.Instance = ADC1;

    // Call callback - should process first half buffer (0 to halfLen=20)
    HAL_ADC_ConvHalfCpltCallback(&mock_adc_correct);

    // Get values after callback
    inputGetJoyAxis(2, &joy_after);

    // Values should be updated through EMA filtering
    // EMA formula: y = y + alpha*(x - y)
    // ALPHA_Q15 = 23675 (≈0.72 in Q15 format)
    // Initial EMA values: emaX = emaY = 2048 (center)
    // Injected averages: avgX = 3000, avgY = 1500

    // Calculate expected X value:
    uint16_t avgX = 3000;
    uint16_t initialX = 2048;
    int32_t errX = (int32_t) avgX - (int32_t) initialX;            // 952
    int32_t deltaX = ((int32_t) 23675 * errX) >> 15;               // 687
    uint16_t expectedX = (uint16_t) ((int32_t) initialX + deltaX); // 2735

    // Calculate expected Y value:
    uint16_t avgY = 1500;
    uint16_t initialY = 2048;
    int32_t errY = (int32_t) avgY - (int32_t) initialY;            // -548
    int32_t deltaY = ((int32_t) 23675 * errY) >> 15;               // -395
    uint16_t expectedY = (uint16_t) ((int32_t) initialY + deltaY); // 1653

    // Verify exact calculated values
    TEST_ASSERT_EQUAL(expectedX, joy_after.jyX); // Should be 2735
    TEST_ASSERT_EQUAL(expectedY, joy_after.jyY); // Should be 1653
}

/**** HAL_ADC_ConvCpltCallback ****/

/**
 * @brief Test HAL_ADC_ConvCpltCallback with wrong ADC instance
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly ignores callbacks
 * from ADC instances other than ADC1. Joystick values should remain unchanged.
 *
 * @test Calls HAL_ADC_ConvCpltCallback with mock ADC instance != ADC1
 * @expected No joystick value changes, no side effects
 */
void test_HAL_ADC_ConvCpltCallback_wrong_adc_instance_does_nothing(void) {
    ADC_HandleTypeDef mock_adc_wrong = { 0 };
    joystick_t joy_before, joy_after;

    // Get initial values
    inputGetJoyAxis(2, &joy_before);

    // Set up wrong ADC instance (not ADC1)
    mock_adc_wrong.Instance = (void *) 0x12345; // Any value different from ADC1

    // Call callback with wrong ADC instance - should do nothing
    HAL_ADC_ConvCpltCallback(&mock_adc_wrong);

    // Get values after callback
    inputGetJoyAxis(2, &joy_after);

    // Values should remain unchanged
    TEST_ASSERT_EQUAL(joy_before.jyX, joy_after.jyX);
    TEST_ASSERT_EQUAL(joy_before.jyY, joy_after.jyY);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback processing second half of DMA buffer
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly processes the second half
 * of the DMA buffer (elements 20-39) using EMA filtering and calls
 * generateJoysticDirectionEvents. Uses values close to center (no events expected).
 *
 * @test Calls callback with ADC1, DMA buffer filled with X=2200, Y=2300
 * @expected Joystick values updated via EMA: jyX=2157, jyY=2230, HAL_GetTick called
 */
void test_HAL_ADC_ConvCpltCallback_processes_second_half_buffer(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    joystick_t joy_before, joy_after;

    // Get initial values (center values from setUp)
    inputGetJoyAxis(2, &joy_before);

    // Inject specific values into the second half of DMA buffer (elements 20-39)
    // Buffer contains interleaved X,Y samples: X0,Y0,X1,Y1,X2,Y2...
    // Set all X values to 2200 and Y values to 2300 (close to center, no events)
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2200;     // X samples
        dmaBuffer.buff[i + 1] = 2300; // Y samples
    }

    // Set up correct ADC instance (ADC1)
    mock_adc_correct.Instance = ADC1;

    // Mock HAL_GetTick for generateJoysticDirectionEvents
    HAL_GetTick_ExpectAndReturn(1000);

    // Call callback - should process second half buffer (20 to 40)
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Get values after callback
    inputGetJoyAxis(2, &joy_after);

    // Values should be updated through EMA filtering
    // EMA formula: y = y + alpha*(x - y)
    // ALPHA_Q15 = 23675 (≈0.72 in Q15 format)
    // Initial EMA values: emaX = emaY = 2048 (center)
    // Injected averages: avgX = 2200, avgY = 2300

    // Calculate expected X value:
    uint16_t avgX = 2200;
    uint16_t initialX = 2048;
    int32_t errX = (int32_t) avgX - (int32_t) initialX;            // 152
    int32_t deltaX = ((int32_t) 23675 * errX) >> 15;               // 109
    uint16_t expectedX = (uint16_t) ((int32_t) initialX + deltaX); // 2157

    // Calculate expected Y value:
    uint16_t avgY = 2300;
    uint16_t initialY = 2048;
    int32_t errY = (int32_t) avgY - (int32_t) initialY;            // 252
    int32_t deltaY = ((int32_t) 23675 * errY) >> 15;               // 182
    uint16_t expectedY = (uint16_t) ((int32_t) initialY + deltaY); // 2230

    // Verify exact calculated values
    TEST_ASSERT_EQUAL(expectedX, joy_after.jyX); // Should be 2157
    TEST_ASSERT_EQUAL(expectedY, joy_after.jyY); // Should be 2230
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback with joystick in center position
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly handles center position
 * joystick readings. Center position (radius < R_IN=800) should not generate
 * any direction events.
 *
 * @test Calls callback with DMA buffer filled with center values (X=2048, Y=2048)
 * @expected HAL_GetTick called, no xQueueSendFromISR calls (no events)
 */
void test_HAL_ADC_ConvCpltCallback_center_position_no_events(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };

    // Inject center position values (no events should be generated)
    // Center position: X=2048, Y=2048, radius=0 (< R_IN=800)
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2048;     // X samples = center
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    // Set up correct ADC instance (ADC1)
    mock_adc_correct.Instance = ADC1;

    // Mock HAL_GetTick for generateJoysticDirectionEvents
    HAL_GetTick_ExpectAndReturn(1000);

    // No queue events expected since joystick is in center position

    // Call callback - should process buffer but generate no events
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback generates RIGHT direction event
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly generates INP_JY_RIGHT event
 * when joystick is moved to the right. Uses X=4000 (right offset) which after
 * EMA filtering produces radius > R_IN=800, triggering direction event.
 *
 * @test Calls callback with DMA buffer: X=4000, Y=2048 (right position)
 * @expected xQueueSendFromISR called with INP_JY_RIGHT event (ID=72)
 */
void test_HAL_ADC_ConvCpltCallback_joystick_right_generates_right_event(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };

    // Inject right position values (X > center, Y = center)
    // Need final EMA result with radius > R_IN=800
    // Right position: X=4000, Y=2048 -> after EMA: X≈3440, radius≈1392 (> R_IN=800)
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 4000;     // X samples = large right offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    // Set up correct ADC instance (ADC1)
    mock_adc_correct.Instance = ADC1;

    // Mock HAL_GetTick for generateJoysticDirectionEvents
    HAL_GetTick_ExpectAndReturn(1000);

    // Enable event capture and expect right direction event to be sent
    event_capture_enabled = 1;
    xQueueSendFromISR_AddCallback(capture_queue_event_from_isr);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();

    // Call callback - should generate INP_JY_RIGHT event
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Verify the correct event was captured (INP_JY_RIGHT = 72)
    TEST_ASSERT_EQUAL(72, captured_event);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback generates LEFT direction event
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly generates INP_JY_LEFT event
 * when joystick is moved to the left. Uses X=100 (left offset) which after
 * EMA filtering produces radius > R_IN=800, triggering direction event.
 *
 * @test Calls callback with DMA buffer: X=100, Y=2048 (left position)
 * @expected xQueueSendFromISR called with INP_JY_LEFT event (ID=71)
 */
void test_HAL_ADC_ConvCpltCallback_joystick_left_generates_left_event(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };

    // Inject left position values (X < center, Y = center)
    // Need final EMA result with radius > R_IN=800
    // Left position: X=100, Y=2048 -> after EMA: X≈656, radius≈1392 (> R_IN=800)
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 100;      // X samples = large left offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    // Set up correct ADC instance (ADC1)
    mock_adc_correct.Instance = ADC1;

    // Mock HAL_GetTick for generateJoysticDirectionEvents
    HAL_GetTick_ExpectAndReturn(1000);

    // Enable event capture and expect left direction event to be sent
    event_capture_enabled = 1;
    captured_event = 0; // Reset
    xQueueSendFromISR_AddCallback(capture_queue_event_from_isr);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();

    // Call callback - should generate INP_JY_LEFT event
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Verify the correct event was captured (INP_JY_LEFT = 71)
    TEST_ASSERT_EQUAL(71, captured_event);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback generates UP direction event
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly generates INP_JY_UP event
 * when joystick is moved up. Uses Y=100 (up offset) which after EMA filtering
 * produces radius > R_IN=800, triggering direction event.
 *
 * @test Calls callback with DMA buffer: X=2048, Y=100 (up position)
 * @expected xQueueSendFromISR called with INP_JY_UP event (ID=69)
 */
void test_HAL_ADC_ConvCpltCallback_joystick_up_generates_up_event(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };

    // Inject up position values (X = center, Y < center)
    // Need final EMA result with radius > R_IN=800
    // Up position: X=2048, Y=100 -> after EMA: Y≈656, radius≈1392 (> R_IN=800)
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2048;    // X samples = center
        dmaBuffer.buff[i + 1] = 100; // Y samples = large up offset
    }

    // Set up correct ADC instance (ADC1)
    mock_adc_correct.Instance = ADC1;

    // Mock HAL_GetTick for generateJoysticDirectionEvents
    HAL_GetTick_ExpectAndReturn(1000);

    // Enable event capture and expect up direction event to be sent
    event_capture_enabled = 1;
    captured_event = 0; // Reset
    xQueueSendFromISR_AddCallback(capture_queue_event_from_isr);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();

    // Call callback - should generate INP_JY_UP event
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Verify the correct event was captured (INP_JY_UP = 69)
    TEST_ASSERT_EQUAL(69, captured_event);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback generates DOWN direction event
 *
 * Verifies that HAL_ADC_ConvCpltCallback correctly generates INP_JY_DOWN event
 * when joystick is moved down. Uses Y=4000 (down offset) which after EMA filtering
 * produces radius > R_IN=800, triggering direction event.
 *
 * @test Calls callback with DMA buffer: X=2048, Y=4000 (down position)
 * @expected xQueueSendFromISR called with INP_JY_DOWN event (ID=70)
 */
void test_HAL_ADC_ConvCpltCallback_joystick_down_generates_down_event(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };

    // Inject down position values (X = center, Y > center)
    // Need final EMA result with radius > R_IN=800
    // Down position: X=2048, Y=4000 -> after EMA: Y≈3440, radius≈1392 (> R_IN=800)
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2048;     // X samples = center
        dmaBuffer.buff[i + 1] = 4000; // Y samples = large down offset
    }

    // Set up correct ADC instance (ADC1)
    mock_adc_correct.Instance = ADC1;

    // Mock HAL_GetTick for generateJoysticDirectionEvents
    HAL_GetTick_ExpectAndReturn(1000);

    // Enable event capture and expect down direction event to be sent
    event_capture_enabled = 1;
    xQueueSendFromISR_AddCallback(capture_queue_event_from_isr);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();

    // Call callback - should generate INP_JY_DOWN event
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Verify the correct event was captured (INP_JY_DOWN = 70)
    TEST_ASSERT_EQUAL(70, captured_event);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback joystick repeat event mechanism
 *
 * Verifies the joystick state machine for repeat events. Tests the transition
 * sequence: JY_CENTER → JY_ACTIVE → JY_REPEAT with correct timing.
 * Initial event sent immediately, repeat events only after 500ms delay.
 *
 * @test Three-step sequence: right movement, same position <500ms, same position ≥500ms
 * @expected First call sends event, second call no event, third call sends repeat event
 */
void test_HAL_ADC_ConvCpltCallback_joystick_repeat_events_after_delay(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    mock_adc_correct.Instance = ADC1;

    // Step 1: Move joystick right to enter ACTIVE state
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 4000;     // X samples = large right offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    // First call - should transition JY_CENTER -> JY_ACTIVE and send initial event
    event_capture_enabled = 1;
    xQueueSendFromISR_AddCallback(capture_queue_event_from_isr);
    HAL_GetTick_ExpectAndReturn(1000);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
    TEST_ASSERT_EQUAL(72, captured_event); // INP_JY_RIGHT

    // Step 2: Call again with same position but before repeat time (< 500ms)
    // Should transition JY_ACTIVE -> JY_REPEAT but NOT send event yet
    HAL_GetTick_ExpectAndReturn(1200); // 200ms later (< 500ms)
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Step 3: Call again after repeat interval (>= 500ms)
    // Should stay in JY_REPEAT and send repeat event
    HAL_GetTick_ExpectAndReturn(1600); // 600ms from initial, >= 500ms from first repeat
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
    TEST_ASSERT_EQUAL(72, captured_event); // INP_JY_RIGHT (repeat)
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback ACTIVE to CENTER state transition
 *
 * Verifies the joystick state machine transition from JY_ACTIVE to JY_CENTER.
 * When joystick returns to center position (radius ≤ R_OUT=600) from active state,
 * no event should be generated.
 *
 * @test Two-step: move right (JY_ACTIVE), then return to center
 * @expected First call sends RIGHT event, second call sends no event
 */
void test_HAL_ADC_ConvCpltCallback_active_to_center_transition(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    mock_adc_correct.Instance = ADC1;

    // Step 1: Move joystick right to enter ACTIVE state
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 4000;     // X samples = large right offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    event_capture_enabled = 1;
    xQueueSendFromISR_AddCallback(capture_queue_event_from_isr);
    HAL_GetTick_ExpectAndReturn(1000);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
    TEST_ASSERT_EQUAL(72, captured_event); // INP_JY_RIGHT

    // Step 2: Move joystick back to center (radius <= R_OUT=600)
    // Should transition JY_ACTIVE -> JY_CENTER and NOT send event
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2048;     // X samples = center
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    HAL_GetTick_ExpectAndReturn(1100);
    // No xQueueSendFromISR expected - returning to center doesn't generate events
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback REPEAT to CENTER state transition
 *
 * Verifies the joystick state machine transition from JY_REPEAT to JY_CENTER.
 * When joystick returns to center position from repeat state, no event should
 * be generated.
 *
 * @test Three-step: move right (JY_ACTIVE), hold (JY_REPEAT), return to center
 * @expected First call sends event, second call no event, third call no event
 */
void test_HAL_ADC_ConvCpltCallback_repeat_to_center_transition(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    mock_adc_correct.Instance = ADC1;

    // Step 1: Move joystick right to enter ACTIVE state
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 4000;     // X samples = large right offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    HAL_GetTick_ExpectAndReturn(1000);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Step 2: Same position - transition to REPEAT state
    HAL_GetTick_ExpectAndReturn(1200);
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Step 3: Move joystick back to center from REPEAT state
    // Should transition JY_REPEAT -> JY_CENTER and NOT send event
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2048;     // X samples = center
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    HAL_GetTick_ExpectAndReturn(1300);
    // No xQueueSendFromISR expected - returning to center doesn't generate events
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback ACTIVE to ACTIVE direction change
 *
 * Verifies the joystick state machine when direction changes while in JY_ACTIVE state.
 * When joystick moves from one direction to another without returning to center,
 * a new direction event should be sent immediately.
 *
 * @test Two-step: move right (JY_ACTIVE), then move left (still JY_ACTIVE)
 * @expected First call sends RIGHT event, second call sends LEFT event
 */
void test_HAL_ADC_ConvCpltCallback_active_to_active_direction_change(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    mock_adc_correct.Instance = ADC1;

    // Step 1: Move joystick right to enter ACTIVE state
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 4000;     // X samples = large right offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    HAL_GetTick_ExpectAndReturn(1000);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Step 2: Change direction to LEFT while in ACTIVE state
    // Should transition JY_ACTIVE -> JY_ACTIVE and send new LEFT event
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 100;      // X samples = large left offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    HAL_GetTick_ExpectAndReturn(1100);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
}

/**
 * @brief Test HAL_ADC_ConvCpltCallback REPEAT to ACTIVE direction change
 *
 * Verifies the joystick state machine when direction changes while in JY_REPEAT state.
 * When joystick moves from one direction to another without returning to center,
 * should transition to JY_ACTIVE and send new direction event immediately.
 *
 * @test Three-step: move right (JY_ACTIVE), hold (JY_REPEAT), move up (JY_ACTIVE)
 * @expected First call sends RIGHT, second no event, third sends UP event
 */
void test_HAL_ADC_ConvCpltCallback_repeat_to_active_direction_change(void) {
    ADC_HandleTypeDef mock_adc_correct = { 0 };
    mock_adc_correct.Instance = ADC1;

    // Step 1: Move joystick right to enter ACTIVE state
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 4000;     // X samples = large right offset
        dmaBuffer.buff[i + 1] = 2048; // Y samples = center
    }

    HAL_GetTick_ExpectAndReturn(1000);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Step 2: Same position - transition to REPEAT state
    HAL_GetTick_ExpectAndReturn(1200);
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);

    // Step 3: Change direction to UP while in REPEAT state
    // Should transition JY_REPEAT -> JY_ACTIVE and send new UP event
    for (int i = 20; i < 40; i += 2) {
        dmaBuffer.buff[i] = 2048;    // X samples = center
        dmaBuffer.buff[i + 1] = 100; // Y samples = large up offset
    }

    HAL_GetTick_ExpectAndReturn(1300);
    xQueueSendFromISR_ExpectAndReturn(qEvents, NULL, NULL, pdTRUE);
    xQueueSendFromISR_IgnoreArg_pvItemToQueue();
    xQueueSendFromISR_IgnoreArg_pxHigherPriorityTaskWoken();
    HAL_ADC_ConvCpltCallback(&mock_adc_correct);
}


/* === End of source code ====================================================================== */