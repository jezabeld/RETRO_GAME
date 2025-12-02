/**
 * @file InputDrv.c
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

/* === Headers files inclusions ================================================================ */
#include "InputDrv.h"
#include "synchronization.h"
#include "main.h"

/* === Macros definitions ====================================================================== */
#define DEBOUNCE_BTN_MS 25 ///< Button debounce time in milliseconds
#define ACTIVE_LOW_BTN 1   ///< Button active logic (1=active low, 0=active high)
#define JY_CENTER_X 2048u  ///< Joystick X-axis center value for 12-bit ADC
#define JY_CENTER_Y 2048u  ///< Joystick Y-axis center value for 12-bit ADC
#define ADC_FS_HZ 1000u    ///< ADC sampling frequency in Hz (X,Y sequence rate)
#define ADC_MAX_12B 4095u  ///< Maximum value for 12-bit ADC
#define ALPHA_Q15 23675u   ///< EMA filter alpha coefficient in Q15 format (≈0.72)

#define R_IN 800u         ///< Joystick inner radius threshold for entering active state
#define R_OUT 600u        ///< Joystick outer radius threshold for returning to center (hysteresis)
#define DOM_MARGIN 50     ///< Dominance margin for axis selection (~1% of ADC range)
#define JY_REPEAT_MS 500u ///< Joystick repeat interval in milliseconds for auto-repeat functionality

/* === Private data type declarations ========================================================== */

/**
 * @brief DMA buffer configuration for joystick ADC data
 */
typedef struct {
    uint16_t buff[40]; ///< DMA buffer for ADC data (20ms @1kHz, 2 channels = 40 half-words)
    uint32_t fullLen;  ///< Total length of DMA buffer
    uint32_t halfLen;  ///< Half buffer length for half/complete callbacks
} dmaBuffer_t;

/**
 * @brief Button finite state machine states
 */
typedef enum {
    BTN_IDLE,          ///< Button not pressed, waiting for falling edge
    BTN_PRESS_PENDING, ///< Edge detected, debouncing in progress
    BTN_PRESSED        ///< Press confirmed (future: hold/release states here)
} btnFSM_t;

/**
 * @brief Button information structure
 */
typedef struct {
    btnFSM_t state;         ///< Current button state
    GPIO_TypeDef *port;     ///< GPIO port
    uint16_t pin;           ///< GPIO pin
    IRQn_Type irqn;         ///< IRQ number
    uint32_t debounceStart; ///< Timestamp when debounce started
    event_id_t pressEvent;  ///< Event to send when button is pressed
    TimerHandle_t timer;    ///< Debounce timer (initially NULL)
} btnInfo_t;

/**
 * @brief Joystick directional finite state machine states
 */
typedef enum {
    JY_CENTER, ///< In central deadzone
    JY_ACTIVE, ///< Outside deadzone, active, changing direction
    JY_REPEAT  ///< Joystick repeating the same direction
} joyFSM_t;

/**
 * @brief Joystick direction key enumeration
 */
typedef enum {
    JOY_KEY_NONE = 0, ///< No direction (centered)
    JOY_KEY_LEFT,     ///< Left direction
    JOY_KEY_RIGHT,    ///< Right direction
    JOY_KEY_UP,       ///< Up direction
    JOY_KEY_DOWN      ///< Down direction
} joyKey_t;

/**
 * @brief Joystick state information structure
 */
typedef struct {
    joyFSM_t state;          ///< Current FSM state
    joyKey_t heldKey;        ///< Current held direction key
    uint32_t next_repeat_at; ///< Next repeat timestamp (using HAL_GetTick)
    uint32_t updated_at;     ///< Last update timestamp
} joyInfo_t;

/**
 * @brief EMA filter state and processing data
 */
typedef struct {
    volatile uint16_t procX, procY; ///< Last processed data values
    uint32_t emaX, emaY;            ///< EMA accumulator values (integer)
} joystickFilter_t;

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */
static void processButtonDebounce(btnInfo_t *btn, const char *btnName);
static void processJoystickFiltering(const uint16_t *buf, uint32_t base, uint32_t half);
static void updateJoystickPublishedValues(void);
static void generateJoysticDirectionEvents(void);
static uint32_t calculateRadius(uint16_t x, uint16_t y);
static uint32_t calculateDistance(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static joyKey_t getDominantDirection(uint16_t x, uint16_t y, joyKey_t prev);

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/** External timer handles for button debounce */
extern TimerHandle_t tBtnAdebounce;
extern TimerHandle_t tBtnBdebounce;
extern TimerHandle_t tBtnCdebounce;
extern TimerHandle_t tBtnDdebounce;

/** DMA buffer instance for joystick ADC data */
dmaBuffer_t dmaBuffer = {
    .buff = { 0 },
    .fullLen = 40, ///< Total buffer length (40 half-words)
    .halfLen = 20  ///< Half buffer length (20 half-words)
};

/** Joystick EMA filter state */
static joystickFilter_t joyFilter = {
    .procX = 0,
    .procY = 0,
    .emaX = 0,
    .emaY = 0,
};

/** Published joystick values accessible via inputGetJoyAxis() */
static volatile joystick_t publishedJoy = { 0 };

/** Joystick directional state machine status */
static joyInfo_t sJoyStatus = { 0 };

/** Button information array for all 4 buttons */
static btnInfo_t sButtons[4] = { { BTN_IDLE, BTN_A_GPIO_Port, BTN_A_Pin, BTN_A_EXTI_IRQn, 0, INP_BTN_A, NULL },
                                 { BTN_IDLE, BTN_B_GPIO_Port, BTN_B_Pin, BTN_B_EXTI_IRQn, 0, INP_BTN_B, NULL },
                                 { BTN_IDLE, BTN_C_GPIO_Port, BTN_C_Pin, BTN_C_EXTI_IRQn, 0, INP_BTN_C, NULL },
                                 { BTN_IDLE, BTN_D_GPIO_Port, BTN_D_Pin, BTN_D_EXTI_IRQn, 0, INP_BTN_D, NULL } };

/* === Private function implementation ========================================================= */

/**
 * @brief Process button debounce and state machine
 *
 * Verifies if button is still pressed after debounce period and sends button event.
 * Re-enables IRQ and resets button state for next press detection.
 *
 * @param btn Pointer to button information structure
 * @param btnName Button name for debugging (unused in release)
 */
static void processButtonDebounce(btnInfo_t *btn, const char *btnName) {
    if (btn->state == BTN_PRESS_PENDING) {
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(btn->port, btn->pin);

        uint8_t is_pressed = (ACTIVE_LOW_BTN) ? (pin_state == GPIO_PIN_RESET) : (pin_state == GPIO_PIN_SET);

        if (is_pressed) {
            btn->state = BTN_PRESSED;
            xQueueSend(qEvents, &(btn->pressEvent), 0);
        } else {
            btn->state = BTN_IDLE;
        }
    }

    __HAL_GPIO_EXTI_CLEAR_IT(btn->irqn);
    HAL_NVIC_EnableIRQ(btn->irqn);
    btn->state = BTN_IDLE;
}

/**
 * @brief Process joystick ADC data with averaging and EMA filtering
 *
 * Averages ADC samples from the specified buffer range and applies EMA filtering
 * to reduce noise and provide smooth joystick readings.
 *
 * @param buf Pointer to ADC buffer containing interleaved X,Y samples
 * @param base Starting index in buffer
 * @param half Number of sample pairs to process
 */
static void processJoystickFiltering(const uint16_t *buf, uint32_t base, uint32_t half) {
    uint32_t sumx = 0, sumy = 0, n = 0;
    uint32_t end = base + half;
    for (uint32_t i = base; i < end; i += 2) { ///< step=2: X,Y interleaved
        sumx += buf[i + 0];
        sumy += buf[i + 1];
        n++;
    }

    if (n == 0) return;

    uint16_t avgx = (uint16_t) (sumx / n);
    uint16_t avgy = (uint16_t) (sumy / n);

    // EMA filtering: y = y + alpha*(x - y)
    int32_t errX = (int32_t) avgx - (int32_t) joyFilter.emaX;
    int32_t deltaX = ((int32_t) ALPHA_Q15 * errX) >> 15;
    int32_t yX = (int32_t) joyFilter.emaX + deltaX;
    if (yX < 0) yX = 0;
    if (yX > (int32_t) ADC_MAX_12B) yX = ADC_MAX_12B;
    joyFilter.emaX = (uint32_t) yX;
    joyFilter.procX = (uint16_t) yX;

    int32_t errY = (int32_t) avgy - (int32_t) joyFilter.emaY;
    int32_t deltaY = ((int32_t) ALPHA_Q15 * errY) >> 15;
    int32_t yY = (int32_t) joyFilter.emaY + deltaY;
    if (yY < 0) yY = 0;
    if (yY > (int32_t) ADC_MAX_12B) yY = ADC_MAX_12B;
    joyFilter.emaY = (uint32_t) yY;
    joyFilter.procY = (uint16_t) yY;
}

/**
 * @brief Update published joystick position values with normalization
 *
 * Normalizes the latest filtered joystick values from ADC range (0-4095)
 * to -1024..+1023 range (2048 values) and publishes them for access via inputGetJoyAxis().
 */
static void updateJoystickPublishedValues(void) {
    // Normalize ADC values (0-4095) to -1024..+1023 range
    // Center (2048) → 0, Min (0) → -1024, Max (4095) → +1023
    int32_t normX = (((int32_t)joyFilter.procX - (int32_t)JY_CENTER_X) * 2048) / (int32_t)ADC_MAX_12B;
    int32_t normY = (((int32_t)joyFilter.procY - (int32_t)JY_CENTER_Y) * 2048) / (int32_t)ADC_MAX_12B;

    // Clamp to -1024..+1023
    if (normX < -1024) normX = -1024;
    if (normX > 1023) normX = 1023;
    if (normY < -1024) normY = -1024;
    if (normY > 1023) normY = 1023;

    publishedJoy.jyX = (int16_t)normX;
    publishedJoy.jyY = (int16_t)normY;
}

/**
 * @brief Generate joystick directional events based on state machine
 *
 * Processes the joystick directional state machine at 50Hz, handling state
 * transitions and generating directional events (INP_JY_UP, DOWN, LEFT, RIGHT)
 * with auto-repeat functionality. Uses raw ADC values for directional detection.
 */
static void generateJoysticDirectionEvents(void) {
    // Use raw ADC values for directional state machine (not normalized values)
    uint32_t r = calculateRadius(joyFilter.procX, joyFilter.procY);
    sJoyStatus.updated_at = HAL_GetTick();
    uint8_t shouldPublish = 0;
    joyKey_t k = getDominantDirection(joyFilter.procX, joyFilter.procY, sJoyStatus.heldKey);

    switch (sJoyStatus.state) {
        case JY_CENTER:
            if (r >= R_IN) { ///< transition JY_CENTER -> JY_ACTIVE
                sJoyStatus.state = JY_ACTIVE;
                if (k != JOY_KEY_NONE) {
                    sJoyStatus.heldKey = k;
                    shouldPublish = 1;
                    sJoyStatus.next_repeat_at = sJoyStatus.updated_at + JY_REPEAT_MS;
                }
            }
            break;

        case JY_ACTIVE:
            if (r <= R_OUT) { ///< transition JY_ACTIVE -> JY_CENTER
                sJoyStatus.state = JY_CENTER;
                sJoyStatus.heldKey = JOY_KEY_NONE;
            } else if (k != sJoyStatus.heldKey) { ///< transition JY_ACTIVE -> JY_ACTIVE
                sJoyStatus.heldKey = k;
                shouldPublish = 1;
                sJoyStatus.next_repeat_at = sJoyStatus.updated_at + JY_REPEAT_MS;
            } else {
                sJoyStatus.state = JY_REPEAT;
            }
            break;

        case JY_REPEAT:
            if (r <= R_OUT) { ///< transition JY_REPEAT -> JY_CENTER
                sJoyStatus.state = JY_CENTER;
                sJoyStatus.heldKey = JOY_KEY_NONE;
            } else if (k != sJoyStatus.heldKey) { ///< transition JY_REPEAT -> JY_ACTIVE
                sJoyStatus.state = JY_ACTIVE;
                sJoyStatus.heldKey = k;
                shouldPublish = 1;
                sJoyStatus.next_repeat_at = sJoyStatus.updated_at + JY_REPEAT_MS;
            } else { ///< transition JY_REPEAT -> JY_REPEAT
                if (sJoyStatus.updated_at >= sJoyStatus.next_repeat_at) {
                    shouldPublish = 1;
                    sJoyStatus.next_repeat_at = sJoyStatus.updated_at + JY_REPEAT_MS;
                }
            }
    }

    if (shouldPublish) {
        event_id_t event;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        switch (sJoyStatus.heldKey) {
            case JOY_KEY_LEFT: event = INP_JY_LEFT; break;
            case JOY_KEY_RIGHT: event = INP_JY_RIGHT; break;
            case JOY_KEY_UP: event = INP_JY_UP; break;
            case JOY_KEY_DOWN: event = INP_JY_DOWN; break;
            default: return;
        }
        xQueueSendFromISR(qEvents, &event, &xHigherPriorityTaskWoken);
    }
}

/**
 * @brief Calculate radius from joystick center
 *
 * @param x Joystick X coordinate
 * @param y Joystick Y coordinate
 * @return Distance from center point
 */
static uint32_t calculateRadius(uint16_t x, uint16_t y) {
    return calculateDistance(x, y, JY_CENTER_X, JY_CENTER_Y);
}

/**
 * @brief Calculate distance between two points using Manhattan approximation
 *
 * Uses fast approximation: distance ≈ max(dx,dy) + 0.5*min(dx,dy)
 *
 * @param x1 First point X coordinate
 * @param y1 First point Y coordinate
 * @param x2 Second point X coordinate
 * @param y2 Second point Y coordinate
 * @return Approximate distance between points
 */
static uint32_t calculateDistance(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    int32_t dx = (int32_t) x1 - (int32_t) x2;
    int32_t dy = (int32_t) y1 - (int32_t) y2;
    uint32_t abs_dx = (dx >= 0) ? (uint32_t) dx : (uint32_t) (-dx);
    uint32_t abs_dy = (dy >= 0) ? (uint32_t) dy : (uint32_t) (-dy);
    uint32_t max_val = (abs_dx > abs_dy) ? abs_dx : abs_dy;
    uint32_t min_val = (abs_dx < abs_dy) ? abs_dx : abs_dy;
    return max_val + (min_val >> 1);
}

/**
 * @brief Determine dominant joystick direction with hysteresis
 *
 * Analyzes joystick position to determine the dominant direction, considering
 * hysteresis and the previous direction to avoid oscillation between similar
 * axis values.
 *
 * @param x Joystick X coordinate
 * @param y Joystick Y coordinate
 * @param prev Previous held direction key
 * @return Dominant direction key or JOY_KEY_NONE if in deadzone
 */
static joyKey_t getDominantDirection(uint16_t x, uint16_t y, joyKey_t prev) {
    int dx = (int) x - (int) JY_CENTER_X;
    int dy = (int) y - (int) JY_CENTER_Y;

    uint32_t r = calculateRadius(x, y);
    if (r <= R_OUT) return JOY_KEY_NONE;

    int ax = (dx >= 0) ? dx : -dx;
    int ay = (dy >= 0) ? dy : -dy;
    int diff = (ax > ay) ? (ax - ay) : (ay - ax);

    if (diff < DOM_MARGIN && prev != JOY_KEY_NONE) return prev;

    if (ax >= ay)
        return (dx >= 0) ? JOY_KEY_RIGHT : JOY_KEY_LEFT;
    else
        return (dy >= 0) ? JOY_KEY_UP : JOY_KEY_DOWN;
}

/* === Public function implementation ========================================================== */

uint8_t inputGetJoyAxis(uint8_t axis, joystick_t *joyPtr) {
    if ((joyPtr == NULL) | (axis > 2)) {
        return 1;
    }

    if (axis == 0) {
        joyPtr->jyX = publishedJoy.jyX;
    } else if (axis == 1) {
        joyPtr->jyY = publishedJoy.jyY;
    } else {
        joyPtr->jyX = publishedJoy.jyX;
        joyPtr->jyY = publishedJoy.jyY;
    }

    return 0;
}

uint8_t inputInit(ADC_HandleTypeDef *adcHandle, DMA_HandleTypeDef *dmaHandle, TIM_HandleTypeDef *timHandle) {
    if (!adcHandle || !dmaHandle || !timHandle) {
        return 1;
    }

    sButtons[0].timer = tBtnAdebounce;
    sButtons[1].timer = tBtnBdebounce;
    sButtons[2].timer = tBtnCdebounce;
    sButtons[3].timer = tBtnDdebounce;

    joyFilter.emaX = JY_CENTER_X;
    joyFilter.emaY = JY_CENTER_Y;
    joyFilter.procX = JY_CENTER_X;
    joyFilter.procY = JY_CENTER_Y;
    publishedJoy.jyX = JY_CENTER_X;
    publishedJoy.jyY = JY_CENTER_Y;

    sJoyStatus.state = JY_CENTER;
    sJoyStatus.heldKey = JOY_KEY_NONE;
    sJoyStatus.next_repeat_at = 0;

    if (HAL_ADC_Start_DMA(adcHandle, (uint32_t *) dmaBuffer.buff, dmaBuffer.fullLen) != HAL_OK) {
        return 1;
    }

    if (HAL_TIM_Base_Start(timHandle) != HAL_OK) {
        return 1;
    }

    return 0;
}

void btnAtimerCallback(TimerHandle_t hTimer) {
    processButtonDebounce(&sButtons[0], "btnA");
}
void btnBtimerCallback(TimerHandle_t hTimer) {
    processButtonDebounce(&sButtons[1], "btnB");
}
void btnCtimerCallback(TimerHandle_t hTimer) {
    processButtonDebounce(&sButtons[2], "btnC");
}
void btnDtimerCallback(TimerHandle_t hTimer) {
    processButtonDebounce(&sButtons[3], "btnD");
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch (GPIO_Pin) { ///< transition BTN_IDLE -> BTN_PRESS_PENDING
        case BTN_A_Pin:
            sButtons[0].state = BTN_PRESS_PENDING;
            xTimerStartFromISR(tBtnAdebounce, &xHigherPriorityTaskWoken);
            HAL_NVIC_DisableIRQ(BTN_A_EXTI_IRQn);
            break;
        case BTN_B_Pin:
            sButtons[1].state = BTN_PRESS_PENDING;
            xTimerStartFromISR(tBtnBdebounce, &xHigherPriorityTaskWoken);
            HAL_NVIC_DisableIRQ(BTN_B_EXTI_IRQn);
            break;
        case BTN_C_Pin:
            sButtons[2].state = BTN_PRESS_PENDING;
            xTimerStartFromISR(tBtnCdebounce, &xHigherPriorityTaskWoken);
            HAL_NVIC_DisableIRQ(BTN_C_EXTI_IRQn);
            break;
        case BTN_D_Pin:
            sButtons[3].state = BTN_PRESS_PENDING;
            xTimerStartFromISR(tBtnDdebounce, &xHigherPriorityTaskWoken);
            HAL_NVIC_DisableIRQ(BTN_D_EXTI_IRQn);
            break;
        default: return;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        processJoystickFiltering(dmaBuffer.buff, 0u, dmaBuffer.halfLen);
        updateJoystickPublishedValues();
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        processJoystickFiltering(dmaBuffer.buff, dmaBuffer.halfLen, dmaBuffer.halfLen);
        updateJoystickPublishedValues();
        generateJoysticDirectionEvents();
    }
}

/* === End of source file ====================================================================== */
