/*
 * InputDrv.c
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "InputDrv.h"
#include "synchronization.h"
#include "main.h"
#include "UARTDrv.h"

// ==== Parámetros ====
#define DEBOUNCE_BTN_MS   25      // 20–30 ms suele ir bien
#define ACTIVE_LOW_BTN    1       // botones con pull-up → nivel bajo = pulsado
#define JY_CENTER_X   2048u    /* 12-bit ADC center value */
#define JY_CENTER_Y   2048u    /* 12-bit ADC center value */
#define JY_DEADZONE   800     /* Dead zone around center */
#define DEBOUNCE_JY_MS 200  /* Debounce time in ms */
#define ADC_FS_HZ    1000u    // 1 kHz por secuencia X->Y
#define ADC_MAX_12B      4095u
#define ALPHA_Q15      23675u   /* ≈0.72 en Q15 → EMA suave y ágil                */

// ==== Estados del botón ====
typedef enum {
    BTN_IDLE,           // No presionado, esperando flanco descendente
    BTN_PRESS_PENDING,  // Flanco detectado, debouncing en curso
    BTN_PRESSED         // Press confirmado (futuro: hold/release aquí)
} btnState_t;

// ==== Información por botón ====
typedef struct {
    btnState_t state;           // Estado actual
    GPIO_TypeDef* port;         // Puerto GPIO
    uint16_t pin;               // Pin GPIO
    IRQn_Type irqn;             // IRQ number
    uint32_t debounceStart;     // Timestamp cuando empezó debounce
    event_id_t pressEvent;      // Evento a enviar
    TimerHandle_t timer;        // Timer de debounce (será NULL inicialmente)
} btnInfo_t;

// ==== Variables externas de timers y configuración ====
extern TimerHandle_t tBtnAdebounce;
extern TimerHandle_t tBtnBdebounce;
extern TimerHandle_t tBtnCdebounce;
extern TimerHandle_t tBtnDdebounce;

/* === Estructuras internas del driver ===================================== */

/* Buffer y configuración DMA */
typedef struct {
    uint16_t buff[40];        /* 20 ms @1 kHz, 2 canales → 40 half-words */
    uint32_t fullLen;         /* longitud total del buffer DMA */
    uint32_t halfLen;         /* mitad del buffer para half/complete callbacks */
} dmaBuffer_t;

/* Estado del filtro EMA y procesamiento */
typedef struct {
    volatile uint16_t avgX, avgY;       /* promedio del bloque actual (crudo) */
    volatile uint16_t procX, procY;     /* últimos datos procesados */
    uint32_t emaX, emaY;                /* acumuladores EMA (entero) */
} joystickFilter_t;

/* Instancias estáticas */
static dmaBuffer_t sDmaBuffer = {
    .buff = {0},
    .fullLen = 40,
    .halfLen = 20
};

static joystickFilter_t sJoyFilter = {
    .avgX = 0, .avgY = 0,
    .procX = 0, .procY = 0,
    .emaX = 0, .emaY = 0
};

/* Variable de publicación */
static volatile joystick_t sPublishedJoy = {0};

static void processJoystickRaw(const uint16_t *buf, uint32_t base, uint32_t half);
static void publishFiltered(void);

// ==== Array de información de botones ====
static btnInfo_t sButtons[4] = {
    {BTN_IDLE, BTN_A_GPIO_Port, BTN_A_Pin, BTN_A_EXTI_IRQn, 0, INP_BTN_A, NULL}, // BTN_A
    {BTN_IDLE, BTN_B_GPIO_Port, BTN_B_Pin, BTN_B_EXTI_IRQn, 0, INP_BTN_B, NULL}, // BTN_B
    {BTN_IDLE, BTN_C_GPIO_Port, BTN_C_Pin, BTN_C_EXTI_IRQn, 0, INP_BTN_C, NULL}, // BTN_C
    {BTN_IDLE, BTN_D_GPIO_Port, BTN_D_Pin, BTN_D_EXTI_IRQn, 0, INP_BTN_D, NULL}  // BTN_D
};

// ==== Funciones helper ====
static void handleButtonDebounce(btnInfo_t* btn, const char* btnName) {
    // Máquina de estados: verificar si el botón sigue presionado
    if (btn->state == BTN_PRESS_PENDING) {
      // Leer el estado actual del pin
      GPIO_PinState pin_state = HAL_GPIO_ReadPin(btn->port, btn->pin);

      // Verificar si está presionado considerando ACTIVE_LOW
      uint8_t is_pressed = (ACTIVE_LOW_BTN) ? (pin_state == GPIO_PIN_RESET) : (pin_state == GPIO_PIN_SET);

      if (is_pressed) {
        btn->state = BTN_PRESSED;
        // Enviar evento de press confirmado
        xQueueSend(qEvents, &(btn->pressEvent), 0);
      } else {
        // Falsa alarma, botón no está presionado
        btn->state = BTN_IDLE;
      }
    }
    
    // Limpia cualquier pending y re-habilita IRQ
    __HAL_GPIO_EXTI_CLEAR_IT(btn->irqn);
    HAL_NVIC_EnableIRQ(btn->irqn);
    btn->state = BTN_IDLE;  // Reset para próximo press
}

uint8_t inputGetJoyAxis(uint8_t axis, joystick_t* joyPtr)
{
    // Verificaciones
    if ((joyPtr == NULL) | (axis > 1)) {
        return 1;  // Error
    }

    if (axis == 0) {
    	joyPtr->jyX = sPublishedJoy.jyX;
    } else {
    	joyPtr->jyY = sPublishedJoy.jyY;
    }

    return 0;
}

uint8_t inputInit(ADC_HandleTypeDef* adcHandle, DMA_HandleTypeDef* dmaHandle, TIM_HandleTypeDef* timHandle)
{
    // Validar parámetros
    if (!adcHandle || !dmaHandle || !timHandle) {
        return 1; // Error: parámetros nulos
    }

    /* Asignar timers a los botones (ya inicializados estáticamente) */
    sButtons[0].timer = tBtnAdebounce;
    sButtons[1].timer = tBtnBdebounce;
    sButtons[2].timer = tBtnCdebounce;
    sButtons[3].timer = tBtnDdebounce;

    /* EMA arranca en el centro típico (si calibrás, poné tus centros aquí) */
    sJoyFilter.emaX = JY_CENTER_X;
    sJoyFilter.emaY = JY_CENTER_Y;
    sJoyFilter.procX = (uint16_t)sJoyFilter.emaX;
    sJoyFilter.procY = (uint16_t)sJoyFilter.emaY;
    sPublishedJoy.jyX = (uint16_t)sJoyFilter.emaX;
    sPublishedJoy.jyY = (uint16_t)sJoyFilter.emaY;

    /* Start ADC + DMA en circular */
    HAL_ADC_Start_DMA(adcHandle, (uint32_t*)sDmaBuffer.buff, sDmaBuffer.fullLen);

    /* Timer maestro del ADC */
    HAL_TIM_Base_Start(timHandle);

    return 0;
}

void btnAtimerCallback(TimerHandle_t hTimer){
    handleButtonDebounce(&sButtons[0], "btnA");
}
void btnBtimerCallback(TimerHandle_t hTimer){
    handleButtonDebounce(&sButtons[1], "btnB");
}
void btnCtimerCallback(TimerHandle_t hTimer){
    handleButtonDebounce(&sButtons[2], "btnC");
}
void btnDtimerCallback(TimerHandle_t hTimer){
    handleButtonDebounce(&sButtons[3], "btnD");
}
/*-----------------------------------------------------------------------
 * Callback común de los 4 botones (PC0…PC3)
 *---------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	switch (GPIO_Pin) {
	case BTN_A_Pin:
		sButtons[0].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnAdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_A_EXTI_IRQn);
		break;
	case BTN_B_Pin:
		sButtons[1].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnBdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_B_EXTI_IRQn);
		break;
	case BTN_C_Pin:
		sButtons[2].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnCdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_C_EXTI_IRQn);
		break;
	case BTN_D_Pin:
		sButtons[3].state = BTN_PRESS_PENDING;  // Transición de estado
		xTimerStartFromISR(tBtnDdebounce,&xHigherPriorityTaskWoken);
		HAL_NVIC_DisableIRQ(BTN_D_EXTI_IRQn);
        break;
    default:
        return;
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* === Helpers ============================================================= */

/* Procesa joystick: promedia muestras y aplica filtro EMA */
static void processJoystickRaw(const uint16_t *buf, uint32_t base, uint32_t half)
{
    // 1. Promedio del bloque de muestras
    uint32_t sumx = 0, sumy = 0, n = 0;
    uint32_t end = base + half;
    for (uint32_t i = base; i < end; i += 2) {   /* paso=2: X,Y interleaved */
        sumx += buf[i + 0];
        sumy += buf[i + 1];
        n++;
    }

    if (n == 0) return;

    uint16_t avgx = (uint16_t)(sumx / n);
    uint16_t avgy = (uint16_t)(sumy / n);

    // Guardar promedio crudo para debug
    sJoyFilter.avgX = avgx;
    sJoyFilter.avgY = avgy;

    // 2. Filtro EMA y guardado en buffer interno
    // EMA X: y = y + alpha*(x - y)
    int32_t errX = (int32_t)avgx - (int32_t)sJoyFilter.emaX;
    int32_t deltaX = ((int32_t)ALPHA_Q15 * errX) >> 15;
    int32_t yX = (int32_t)sJoyFilter.emaX + deltaX;
    if (yX < 0) yX = 0;
    if (yX > (int32_t)ADC_MAX_12B) yX = ADC_MAX_12B;
    sJoyFilter.emaX = (uint32_t)yX;
    sJoyFilter.procX = (uint16_t)yX;  // Guardar en buffer interno

    // EMA Y: y = y + alpha*(x - y)
    int32_t errY = (int32_t)avgy - (int32_t)sJoyFilter.emaY;
    int32_t deltaY = ((int32_t)ALPHA_Q15 * errY) >> 15;
    int32_t yY = (int32_t)sJoyFilter.emaY + deltaY;
    if (yY < 0) yY = 0;
    if (yY > (int32_t)ADC_MAX_12B) yY = ADC_MAX_12B;
    sJoyFilter.emaY = (uint32_t)yY;
    sJoyFilter.procY = (uint16_t)yY;  // Guardar en buffer interno
}

/* Publica los últimos datos procesados disponibles */
static void publishFiltered(void) {
    sPublishedJoy.jyX = sJoyFilter.procX;
    sPublishedJoy.jyY = sJoyFilter.procY;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        // Primera mitad del buffer - procesa datos
        processJoystickRaw(sDmaBuffer.buff, 0u, sDmaBuffer.halfLen);
        // Para salida a 100 Hz: descomentar publishFiltered();
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        // Segunda mitad del buffer - procesa y publica a 50 Hz (20 ms)
        processJoystickRaw(sDmaBuffer.buff, sDmaBuffer.halfLen, sDmaBuffer.halfLen);
        publishFiltered();
    }
}

