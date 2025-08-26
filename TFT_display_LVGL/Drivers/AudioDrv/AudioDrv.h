/*
 * AudioDrv.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#ifndef AUDIODRV_AUDIODRV_H_
#define AUDIODRV_AUDIODRV_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef enum {
    WF_SINE = 0,
    WF_TRI,
    WF_SAW,
    WF_SQUARE,
    WF_NOISE
} wave_t;

#define DMA_BUFFER_SIZE 32
#define SAMPLE_FREQ 10000
#define DAC_OUT_MID 2048 //el DAC es de 12 bits (valores 0-4095)
#define FS_HZ            10000U
//#define DMA_BUFFER_SIZE  64
//#define DAC_MID          2048
#define FADE_SAMPLES     64      // ~6.4 ms a 10 kHz


extern volatile uint16_t dac_dma_buffer[2* DMA_BUFFER_SIZE]; // 2 mitades de 32 bits

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac_);

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac_);

//void Audio_Beep(float freq_hz, uint32_t dur_ms);
void Audio_BeepEx(float freq_hz, uint32_t dur_ms, wave_t wf, uint16_t amp);
void audioInit();
void audio_task(void);

#endif /* AUDIODRV_AUDIODRV_H_ */
