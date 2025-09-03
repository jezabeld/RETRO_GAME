/*
 * AudioDrv.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */


#ifndef DRIVERS_AUDIODRV_H_
#define DRIVERS_AUDIODRV_H_

// #include <stdint.h>
// #include <stdbool.h>

// typedef struct {
//     const int16_t *pcm;
//     uint32_t durationMs;
// } audioPcmCue_t;

// int audioInit(void);
// int audioPlayFx(const audioPcmCue_t *cue);


#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifndef AUDIO_DMA_BLOCK
#define AUDIO_DMA_BLOCK  64u
#endif

#define AUDIO_SAMPLE_MAX     4095u   /* 12-bit samples (mapped to PWM levels) */
#define AUDIO_SAMPLE_CENTER  2048u   /* Sample center value (50% duty) */

/* Callback: llená 'dst' con N muestras 12-bit (0..4095, centro=2048)
 * NOTA: PWM real solo tiene niveles limitados, pero se usa 12-bit para
 * compatibilidad futura con DAC y simplicidad del AudioPlayer */
typedef void (*audioPwmFill12Fn)(uint16_t* dst, uint32_t n);

/* Configuración del driver de audio PWM */
typedef struct {
    TIM_HandleTypeDef  *pwm_timer;     /* Timer PWM (ej: TIM3) */
    uint32_t           pwm_channel;    /* Canal PWM (ej: TIM_CHANNEL_3) */
    uint32_t           pwm_arr;        /* ARR del timer PWM */
    
    TIM_HandleTypeDef  *fs_timer;      /* Timer de frecuencia de muestreo (ej: TIM6) */
    DMA_HandleTypeDef  *dma_handle;    /* DMA para transferir samples */
    uint32_t           fs_hz;          /* Frecuencia de muestreo */
    
    volatile uint16_t  pwm_buffer[2 * AUDIO_DMA_BLOCK];  /* Buffer PWM (ping-pong) */
    audioPwmFill12Fn   fill_callback;  /* Callback para generar samples */
} audioDrv_t;


/* Inicializa el driver de audio PWM */
int audioInit(audioDrv_t *audio, TIM_HandleTypeDef *pwm_tim, uint32_t pwm_ch, 
              TIM_HandleTypeDef *fs_tim, DMA_HandleTypeDef *dma, 
              uint32_t fs_hz);

/* Arranca PWM + DMA circular */
void audioStart(audioDrv_t *audio);

/* Detiene limpio (CCR a 50%) */
void audioStop(audioDrv_t *audio);

/* Registra la función de llenado */
void audioSetFillFn(audioDrv_t *audio, audioPwmFill12Fn fn);

/* Lee frecuencia de muestreo */
uint32_t audioGetFs(audioDrv_t *audio);

#endif /* DRIVERS_AUDIODRV_H_ */
