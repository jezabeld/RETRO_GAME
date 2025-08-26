/*
 * mario_sound.h
 *
 *  Created on: Aug 25, 2025
 *      Author: jez
 */

#ifndef AUDIODRV_PLAY_SOUND_H_
#define AUDIODRV_PLAY_SOUND_H_

#include "AudioDrv.h"

#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_DS4  311
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_FS4  370
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440
#define NOTE_AS4  466
#define NOTE_B4   494

#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_DS5  622
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_FS5  740
#define NOTE_G5   784
#define NOTE_GS5  830
#define NOTE_A5   880

#define BPM     120
#define Q_NOTE  (60000/BPM)   // negra




typedef struct {
    float freq;
    uint32_t dur_ms;
    wave_t wf;
    uint16_t amp;
} beep_entry_t;


void Play_Song_Start(const beep_entry_t *song, uint32_t length);
void Play_Song_Update(void);





#endif /* AUDIODRV_PLAY_SOUND_H_ */
