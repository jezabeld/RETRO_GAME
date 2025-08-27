/*
 * audioPwm.h
 *
 *  Created on: Aug 27, 2025
 *      Author: jez
 */

#ifndef AUDIODRV_AUDIOPWM_H_
#define AUDIODRV_AUDIOPWM_H_

#include <stdint.h>

void AudioPWM_Start(float tone_hz, uint32_t fs_hz);
void AudioPWM_Stop(void);

#endif /* AUDIODRV_AUDIOPWM_H_ */
