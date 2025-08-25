/*
 * AudioDrv.c
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#include "AudioDrv.h"
#include <math.h>

static float angle = 0.0f;
static float angle_step = 0.0f;
static float angle_change = 1000 * (2 * M_PI / SAMPLE_FREQ); // each step around the unit circle
static uint32_t t_end_ms = 0;
static volatile uint8_t audio_on = 0;
static volatile uint8_t stop_pending = 0;
static volatile uint32_t fade_left = 0; // contador de samples de fade en curso
volatile uint16_t dac_dma_buffer[2* DMA_BUFFER_SIZE]; // 2 mitades de 32 bits

static inline uint16_t env_scale_sample(uint16_t s);
//static void fill_block(volatile uint16_t *buf, uint32_t n);
static void do_dac(volatile uint16_t *buffer);
static void audio_start(float freq_hz, uint32_t dur_ms);
static void audio_stop_clean(void);
//static void MX_DAC_Init_TrigTIM6(void);
//static void MX_DAC_Init_NoTrigger(void);
//static void Wavetable_Init(void);

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim6;

void audioInit(){
    HAL_TIM_Base_Start(&htim6);
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)&dac_dma_buffer, 2 * DMA_BUFFER_SIZE, DAC_ALIGN_12B_R);
}

static inline uint16_t env_scale_sample(uint16_t s)
{
    if (fade_left == 0) return s;

    // Mezcla lineal hacia el centro (2048):
    // s_out = 2048 + (s_in - 2048) * (fade_left / FADE_SAMPLES)
    // (usar enteros para no “pisar” FPU si no querés)
    int32_t d = (int32_t)s - (int32_t)DAC_OUT_MID;
    int32_t y = (int32_t)DAC_OUT_MID + (d * (int32_t)fade_left) / (int32_t)FADE_SAMPLES;
    return (uint16_t)(y < 0 ? 0 : (y > 4095 ? 4095 : y));
}

static void fill_block(volatile uint16_t *buf, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        // seno 12 bits centrado
        float s = cosf(angle);                         // o sinf, da igual
        uint16_t v = (uint16_t)(DAC_OUT_MID + 1800.0f * s);
        if (fade_left) v = env_scale_sample(v);
        buf[i] = v;

        angle += angle_step;
        if (angle >= 2.0f * (float)M_PI) angle -= 2.0f * (float)M_PI;
    }
    if (fade_left) {
        // Consumí 'n' muestras de fade, sin ir por debajo de cero
        fade_left = (fade_left > n) ? (fade_left - n) : 0;
    }
}
//
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac_)
{
    if (!audio_on) return;
    fill_block(&dac_dma_buffer[0], DMA_BUFFER_SIZE);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac_)
{
    if (!audio_on) return;
    fill_block(&dac_dma_buffer[DMA_BUFFER_SIZE], DMA_BUFFER_SIZE);
}

static void audio_start(float freq_hz, uint32_t dur_ms)
{
    // Paso angular para la frecuencia pedida
    angle_step = 2.0f * (float)M_PI * (freq_hz / (float)FS_HZ);
    angle = 0.0f;

    // Prefill con 2048 para evitar "click" al primer disparo
    for (uint32_t i = 0; i < 2*DMA_BUFFER_SIZE; i++) dac_dma_buffer[i] = DAC_OUT_MID;

    fade_left = 0;
    stop_pending = 0;
    audio_on = 1;
    t_end_ms = HAL_GetTick() + dur_ms;

    // Orden correcto: DAC -> DMA -> TIM (TRGO)
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)&dac_dma_buffer, 2 * DMA_BUFFER_SIZE, DAC_ALIGN_12B_R);
    HAL_TIM_Base_Start(&htim6);  // TIM6 genera el trigger del DAC
}

static void audio_stop_clean(void)
{
    // Paro el trigger primero para congelar el DAC en el último sample ya "fadeado"
    HAL_TIM_Base_Stop(&htim6);
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, DAC_OUT_MID);
    audio_on = 0;
    stop_pending = 0;
    fade_left = 0;
}

void audio_task(void)
{
    if (!audio_on) return;

    // ¿se venció el tiempo? pedí fade-out y luego paro cuando termine
    if (!stop_pending && (int32_t)(HAL_GetTick() - t_end_ms) >= 0) {
        stop_pending = 1;
        fade_left = FADE_SAMPLES;   // arrancá la rampa hacia 2048
    }

    // Cuando el fade terminó (callbacks ya “dibujaron” 2048),
    // es seguro cortar sin click
    if (stop_pending && fade_left == 0) {
        audio_stop_clean();
    }
}

void Audio_Beep(float freq_hz, uint32_t dur_ms)
{
    // si ya estaba sonando, cortá limpio primero
    if (audio_on) { stop_pending = 1; fade_left = FADE_SAMPLES; }
    audio_start(freq_hz, dur_ms);
}

// static void do_dac(volatile uint16_t *buffer) {
// 	for(int i=0; i < DMA_BUFFER_SIZE; i++){
// 		buffer[i] = DAC_OUT_MID - ((DAC_OUT_MID * cos(angle)));
// 		angle += angle_change;
// 		if(angle >= (2*M_PI)){ // si me paso vuelvo entre o-2pi
// 			angle -= (2*M_PI);
// 		}
// 	}
// }
// void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
// 	do_dac(&dac_dma_buffer[DMA_BUFFER_SIZE]);
// }
// void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
// 	do_dac(&dac_dma_buffer[0]);
// }
