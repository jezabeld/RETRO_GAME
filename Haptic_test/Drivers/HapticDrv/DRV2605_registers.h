/*
 * DRV2605_registers.h
 *
 *  Created on: Aug 18, 2025
 *      Author: jez
 */
/*
 * Haptic_DRV2605_registers.h - Haptics REGISTERS for DRV2605
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef HAPTICDRV_DRV2605_REGISTERS_H_
#define HAPTICDRV_DRV2605_REGISTERS_H_

#define DRV2605_ADDR         (0x5Au << 1)

#define DRV2605_REG_STATUS        0x00
#define DRV2605_REG_MODE          0x01

#define DRV2605_REG_RTPIN         0x02
#define DRV2605_REG_LIBRARY       0x03
#define DRV2605_REG_WAVESEQ1      0x04
#define DRV2605_REG_WAVESEQ2      0x05
#define DRV2605_REG_WAVESEQ3      0x06
#define DRV2605_REG_WAVESEQ4      0x07
#define DRV2605_REG_WAVESEQ5      0x08
#define DRV2605_REG_WAVESEQ6      0x09
#define DRV2605_REG_WAVESEQ7      0x0A
#define DRV2605_REG_WAVESEQ8      0x0B

#define DRV2605_REG_GO            0x0C
#define DRV2605_REG_OVERDRIVE     0x0D
#define DRV2605_REG_SUSTAINPOS    0x0E
#define DRV2605_REG_SUSTAINNEG    0x0F
#define DRV2605_REG_BRAKE         0x10
#define DRV2605_REG_AUDIOVIBECTRL 0x11
#define DRV2605_REG_AUDIOMINLVL   0x12
#define DRV2605_REG_AUDIOMAXLVL   0x13
#define DRV2605_REG_AUDIOMINDRV   0x14
#define DRV2605_REG_AUDIOMAXDRV   0x15
#define DRV2605_REG_RATEDV        0x16
#define DRV2605_REG_CLAMPV        0x17
#define DRV2605_REG_AUTOCALCOMP   0x18
#define DRV2605_REG_AUTOCALEMP    0x19
#define DRV2605_REG_FEEDBACK      0x1A
#define DRV2605_REG_CONTROL1      0x1B
#define DRV2605_REG_CONTROL2      0x1C
#define DRV2605_REG_CONTROL3      0x1D
#define DRV2605_REG_CONTROL4      0x1E
#define DRV2605_REG_RFU1          0x1F
#define DRV2605_REG_RFU2          0x20
#define DRV2605_REG_VBAT          0x21
#define DRV2605_REG_LRARESON      0x22

// Registros DRV2605L (principales)
#define DRV2605_REG_STATUS     0x00
#define DRV2605_REG_MODE       0x01
#define DRV2605_REG_RTPIN      0x02
#define DRV2605_REG_LIBRARY    0x03
#define DRV2605_REG_WAVESEQ1   0x04  // ..0x0B WAVESEQ8
#define DRV2605_REG_GO         0x0C
#define DRV2605_REG_FEEDBACK   0x1A
#define DRV2605_REG_CONTROL1   0x1B
#define DRV2605_REG_CONTROL2   0x1C
#define DRV2605_REG_CONTROL3   0x1D
#define DRV2605_REG_CONTROL4   0x1E
#define DRV2605_REG_RATEDV     0x16
#define DRV2605_REG_CLAMPV     0x17

/* Valores de MODE (de Adafruit)
 *    0: Internal trigger, call go() to start playback
 *    1: External trigger, rising edge on IN pin starts playback
 *    2: External trigger, playback follows the state of IN pin
 *    3: PWM/analog input
 *    4: Audio
 *    5: Real-time playback
 *    6: Diagnostics
 *    7: Auto calibration
 */
#define DRV2605_MODE_INTTRIG   0x00   // Internal trigger (register)
#define DRV2605_MODE_EXTTRIG   0x01
#define DRV2605_MODE_RTP       0x02
#define DRV2605_MODE_DFRT      0x03
#define DRV2605_MODE_PWMANALOG 0x05
#define DRV2605_MODE_AUDIOVIBE 0x07
#define DRV2605_MODE_STANDBY   0x40   // bit6

// Librerías típicas
#define DRV2605_LIB_ERM_A      0x01   // buena para ERM “bar/coreless”
#define DRV2605_LIB_ERM_F      0x07   // buena para ERM “coin/flat”

#endif /* HAPTICDRV_DRV2605_REGISTERS_H_ */
