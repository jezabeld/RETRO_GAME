/*
 * AudioDrv.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef DRIVERS_AUDIODRV_H_
#define DRIVERS_AUDIODRV_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const int16_t *pcm;
    uint32_t durationMs;
} audioPcmCue_t;

int audioInit(void);
int audioPlayFx(const audioPcmCue_t *cue);

#endif /* DRIVERS_AUDIODRV_H_ */