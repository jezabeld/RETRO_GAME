/*
 * UARTDrv.c
 *
 * TX con cola de buffers + DMA no bloqueante + RX con ReceiveToIdle DMA
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#include "UARTDrv.h"
#include "string.h"
#include "errno.h"
#include "cmsis_os.h"

#define UART_TX_BUF_SIZE  128
#define UART_RX_BUF_SIZE  128
#define TX_RING_SZ  512

static uint8_t  txRing[TX_RING_SZ];
static volatile uint16_t txHead = 0;
static volatile uint16_t txTail = 0;
static volatile uint16_t lastLen = 0;
static volatile uint8_t  txBusy  = 0;

// ===== Buffer RX para ReceiveToIdle DMA =====
static uint8_t rxBuf[UART_RX_BUF_SIZE];

// Prototipos internos
static void intToStr(int32_t num, char* str);
static void uart_kick_from_tail(void);

// Handlers
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

// ===== Helpers ring =====
static inline uint16_t rb_count(void) {
    uint16_t h = txHead, t = txTail;
    return (uint16_t)((h + TX_RING_SZ - t) % TX_RING_SZ);
}
static inline uint16_t rb_free(void) {
    return (uint16_t)(TX_RING_SZ - 1 - rb_count());
}

// Arranca un DMA con el tramo contiguo disponible desde tail
static void uart_kick_from_tail(void)
{
    // precondición: llamar desde zona crítica
    if (txBusy) return;
    uint16_t t = txTail, h = txHead;
    if (t == h) return; // vacío

    uint16_t len = (h > t) ? (h - t) : (TX_RING_SZ - t);
    lastLen = len;
    txBusy  = 1;
    // fuera de crítico por si HAL tarda; pero los índices están inmutables hasta callback
    taskEXIT_CRITICAL();

    (void)HAL_UART_Transmit_DMA(&huart2, &txRing[t], len);

    // volver a crítico solo si lo necesitas; normalmente no hace falta.
    taskENTER_CRITICAL();
}


// ------------------------------------------------------------
// Inicialización
// ------------------------------------------------------------
int uartInit(void)
{
    // Si NO usas MX_USART2_UART_Init, acá deberías configurar huart2.Init.* y llamar HAL_UART_Init(&huart2)
    // Además, si NO usas CubeMX para enlazar DMA:
    // __HAL_LINKDMA(&huart2, hdmatx, hdma_usart2_tx);
    // __HAL_LINKDMA(&huart2, hdmarx, hdma_usart2_rx);
	// eso esta en Core/Src/stm32f4xx_hal_msp.c
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        return -1;
    }

    // RX: DMA + IDLE (no bloqueante)
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxBuf, UART_RX_BUF_SIZE) != HAL_OK) {
        return -1;
    }

    return 0;
}


// ------------------------------------------------------------
// TX no bloqueante por DMA
// ------------------------------------------------------------
// Función base común para envío de datos
int uartSend(const void *buf, uint16_t len)
{
    if (!buf || !len) return -EINVAL;

    taskENTER_CRITICAL();
    uint16_t free = rb_free();
    if (len > free) { taskEXIT_CRITICAL(); return -ENOSPC; }

    // Copia circular
    const uint8_t *p = (const uint8_t*)buf;
    uint16_t h = txHead;
    uint16_t first = (uint16_t)((TX_RING_SZ - h) < len ? (TX_RING_SZ - h) : len);
    memcpy(&txRing[h], p, first);
    if (len > first) memcpy(&txRing[0], p + first, len - first);
    txHead = (uint16_t)((h + len) % TX_RING_SZ);

    // Si estaba ocioso, arrancar inmediatamente
    if (!txBusy) uart_kick_from_tail();

    taskEXIT_CRITICAL();
    return 0;
}

int uartSendString(const char *s)
{
    return uartSend(s, (uint16_t)strlen(s));
}

// Función para strings con longitud conocida (más eficiente)
int uartSendStringSize(const char *pstring, uint16_t size)
{
    return uartSend(pstring, size);
}

// ------------------------------------------------------------
// RX callback (ReceiveToIdle DMA)
// ------------------------------------------------------------
// HAL llamará esto cada vez que detecte IDLE o se llene el buffer con bytes nuevos.
// 'Size' = cantidad de bytes válidos en rxBuf.
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart != &huart2) return;

    // Entregar a un hook débil (puede reenviar a una cola, parsear comandos, etc.)
    UARTDrv_OnRxData(rxBuf, Size);

    // Rearmar la recepción para seguir escuchando
    (void)HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxBuf, UART_RX_BUF_SIZE);
}

// ------------------------------------------------------------
// TX complete callback (libera el DMA para el próximo envío)
// ------------------------------------------------------------
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart2) return;

    // Avanzar tail por lo que se envió
    taskENTER_CRITICAL_FROM_ISR();
    txTail = (uint16_t)((txTail + lastLen) % TX_RING_SZ);

    // ¿Queda data? lanzar siguiente ahora. Si no, soltar busy.
    uint16_t t = txTail, h = txHead;
    if (t != h) {
        uint16_t len = (h > t) ? (h - t) : (TX_RING_SZ - t);
        lastLen = len;
        // Seguimos en ISR: es válido encadenar otro DMA con HAL
        taskEXIT_CRITICAL_FROM_ISR(0);
        (void)HAL_UART_Transmit_DMA(&huart2, &txRing[t], len);
        // No re-entrar a crítico; el próximo callback nos vuelve a traer aquí.
    } else {
        txBusy = 0;
        taskEXIT_CRITICAL_FROM_ISR(0);
    }
}

// ------------------------------------------------------------
// Utilidad imprimir enteros
// ------------------------------------------------------------
int uartSendValue(int32_t value)
{
    char buffer[12];
    intToStr(value, buffer);
    return uartSend(buffer, (uint16_t)strlen(buffer));
}

static void intToStr(int32_t num, char* str)
{
    uint8_t i = 0, neg = 0;
    if (num == 0) { str[i++] = '0'; }
    if (num < 0) { neg = 1; num = -num; }
    while (num > 0) { str[i++] = (num % 10) + '0'; num /= 10; }
    if (neg) { str[i++] = '-'; }
    str[i] = '\0';
    for (uint8_t j = 0; j < i/2; j++) { char t=str[j]; str[j]=str[i-1-j]; str[i-1-j]=t; }
}

// ------------------------------------------------------------
// Hook débil (puede ser redefinido en otro módulo)
// Por defecto, ignora los datos recibidos.
// ------------------------------------------------------------
__attribute__((weak)) void UARTDrv_OnRxData(const uint8_t *data, uint16_t len)
{
    (void)data; (void)len;
    // Ejemplo (si quisieras eco no bloqueante):
    // uartSendStringSize((const char*)data, len);
}
