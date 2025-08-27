/*
 * mario_sound.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#ifndef AUDIODRV_AUDIOPLAYER_H_
#define AUDIODRV_AUDIOPLAYER_H_

#include <stdint.h>

typedef enum { WF_SINE=0, WF_TRI, WF_SAW, WF_SQUARE } wave_t;

typedef struct {
    uint16_t freq_hz;   // 0 = silencio
    uint16_t dur_ms;
    wave_t   wave;
    uint16_t amp12;     // 0..~2047
} note_t;

/* Inicializa y engancha su fill al driver PWM */
void AudioPlayer_Init(uint32_t fs_hz);

/* Música (loop opcional) */
void AudioPlayer_StartMusic(const note_t* song, uint32_t len, uint8_t loop);

/* Parar todo */
void AudioPlayer_StopAll(void);

/* SFX que pisa la música momentáneamente */
void AudioPlayer_PlaySfx(note_t sfx);

/* Volumen global 0..1 */
void AudioPlayer_SetVolume(float g);


#endif /* AUDIODRV_AUDIOPLAYER_H_ */
