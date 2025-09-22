#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __NOP()                                                                                                        \
    void nop(void) { }
#define HAL_MAX_DELAY 0xFFFFFFFFU

/* Estructura mínima/opiaca: suficiente para punteros */
typedef struct {
    int _unused;
} GPIO_TypeDef;
/* Tipos mínimos para GPIO */
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET } GPIO_PinState;

/* GPIO Pin definitions */
#define GPIO_PIN_0 ((uint16_t) 0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1 ((uint16_t) 0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2 ((uint16_t) 0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3 ((uint16_t) 0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4 ((uint16_t) 0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5 ((uint16_t) 0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6 ((uint16_t) 0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7 ((uint16_t) 0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8 ((uint16_t) 0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9 ((uint16_t) 0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10 ((uint16_t) 0x0400) /* Pin 10 selected   */
#define GPIO_PIN_11 ((uint16_t) 0x0800) /* Pin 11 selected   */
#define GPIO_PIN_12 ((uint16_t) 0x1000) /* Pin 12 selected   */
#define GPIO_PIN_13 ((uint16_t) 0x2000) /* Pin 13 selected   */
#define GPIO_PIN_14 ((uint16_t) 0x4000) /* Pin 14 selected   */
#define GPIO_PIN_15 ((uint16_t) 0x8000) /* Pin 15 selected   */

/* EXTI IRQ Numbers */
typedef enum {
    EXTI0_IRQn = 6,      /*!< EXTI Line0 Interrupt                                              */
    EXTI1_IRQn = 7,      /*!< EXTI Line1 Interrupt                                              */
    EXTI2_IRQn = 8,      /*!< EXTI Line2 Interrupt                                              */
    EXTI3_IRQn = 9,      /*!< EXTI Line3 Interrupt                                              */
    EXTI4_IRQn = 10,     /*!< EXTI Line4 Interrupt                                              */
    EXTI9_5_IRQn = 23,   /*!< External Line[9:5] Interrupts                                     */
    EXTI15_10_IRQn = 40, /*!< External Line[15:10] Interrupts                                   */
} IRQn_Type_Base;

/* GPIO Port instances - defined as constants for static initialization */
#define GPIOA ((GPIO_TypeDef *)0x40020000UL)
#define GPIOB ((GPIO_TypeDef *)0x40020400UL)
#define GPIOC ((GPIO_TypeDef *)0x40020800UL)
#define GPIOD ((GPIO_TypeDef *)0x40020C00UL)
#define GPIOE ((GPIO_TypeDef *)0x40021000UL)
#define GPIOF ((GPIO_TypeDef *)0x40021400UL)
#define GPIOG ((GPIO_TypeDef *)0x40021800UL)
#define GPIOH ((GPIO_TypeDef *)0x40021C00UL)

/* ADC instances */
typedef struct {
    uint32_t dummy;
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *)0x40012000UL)
#define ADC2 ((ADC_TypeDef *)0x40012100UL)
#define ADC3 ((ADC_TypeDef *)0x40012200UL)

#ifdef __cplusplus
}
#endif
