/*
 * AudioPlayer.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#ifndef AUDIODRV_AUDIOPLAYER_H_
#define AUDIODRV_AUDIOPLAYER_H_

#include <stdint.h>
#include "AudioDrv.h"

typedef enum { WF_SINE=0, WF_TRI, WF_SAW, WF_SQUARE } wave_t;

typedef struct {
    uint16_t freq_hz;   // 0 = silencio
    uint16_t dur_ms;
    wave_t   wave;
    uint16_t amp12;     // 0..~2047
} note_t;

/* Inicializa el reproductor de audio (port layer) */
void playerInit(audioDrv_t *audio);

/* Música (loop opcional) */
void playerStartMusic(const note_t* song, uint32_t len, uint8_t loop);

/* Parar todo */
void playerStopAll(void);

/* SFX que pisa la música momentáneamente */
void playerPlaySfx(note_t sfx);

/* Volumen global 0..1 */
void playerSetVolume(float g);


#endif /* AUDIODRV_AUDIOPLAYER_H_ */
