/*
 * DRV2605.c
 *
 *  Created on: Aug 18, 2025
 *      Author: jez
 */


#include <HapticDrv.h>

#ifndef DRV2605_I2C_TIMEOUT_MS
#define DRV2605_I2C_TIMEOUT_MS  10   // ms
#endif

#define DRV2605_ADDR         (0x5Au << 1) ///< Device I2C address

#define DRV2605_REG_STATUS 0x00       ///< Status register
#define DRV2605_REG_MODE 0x01         ///< Mode register
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
#define DRV2605_MODE_INTTRIG 0x00     ///< Internal trigger mode
#define DRV2605_MODE_EXTTRIGEDGE 0x01 ///< External edge trigger mode
#define DRV2605_MODE_EXTTRIGLVL 0x02  ///< External level trigger mode
#define DRV2605_MODE_PWMANALOG 0x03   ///< PWM/Analog input mode
#define DRV2605_MODE_AUDIOVIBE 0x04   ///< Audio-to-vibe mode
#define DRV2605_MODE_REALTIME 0x05    ///< Real-time playback (RTP) mode
#define DRV2605_MODE_DIAGNOS 0x06     ///< Diagnostics mode
#define DRV2605_MODE_AUTOCAL 0x07     ///< Auto calibration mode

#define DRV2605_REG_RTPIN 0x02    ///< Real-time playback input register
#define DRV2605_REG_LIBRARY 0x03  ///< Waveform library selection register
#define DRV2605_REG_WAVESEQ1 0x04 ///< Waveform sequence register 1
#define DRV2605_REG_WAVESEQ2 0x05 ///< Waveform sequence register 2
#define DRV2605_REG_WAVESEQ3 0x06 ///< Waveform sequence register 3
#define DRV2605_REG_WAVESEQ4 0x07 ///< Waveform sequence register 4
#define DRV2605_REG_WAVESEQ5 0x08 ///< Waveform sequence register 5
#define DRV2605_REG_WAVESEQ6 0x09 ///< Waveform sequence register 6
#define DRV2605_REG_WAVESEQ7 0x0A ///< Waveform sequence register 7
#define DRV2605_REG_WAVESEQ8 0x0B ///< Waveform sequence register 8

#define DRV2605_REG_GO 0x0C         ///< Go register
#define DRV2605_REG_OVERDRIVE 0x0D  ///< Overdrive time offset register
#define DRV2605_REG_SUSTAINPOS 0x0E ///< Sustain time offset, positive register
#define DRV2605_REG_SUSTAINNEG 0x0F ///< Sustain time offset, negative register
#define DRV2605_REG_BREAK 0x10      ///< Brake time offset register
#define DRV2605_REG_AUDIOCTRL 0x11  ///< Audio-to-vibe control register
#define DRV2605_REG_AUDIOLVL                                                   \
  0x12 ///< Audio-to-vibe minimum input level register
#define DRV2605_REG_AUDIOMAX                                                   \
  0x13 ///< Audio-to-vibe maximum input level register
#define DRV2605_REG_AUDIOOUTMIN                                                \
  0x14 ///< Audio-to-vibe minimum output drive register
#define DRV2605_REG_AUDIOOUTMAX                                                \
  0x15                          ///< Audio-to-vibe maximum output drive register
#define DRV2605_REG_RATEDV 0x16 ///< Rated voltage register
#define DRV2605_REG_CLAMPV 0x17 ///< Overdrive clamp voltage register
#define DRV2605_REG_AUTOCALCOMP                                                \
  0x18 ///< Auto-calibration compensation result register
#define DRV2605_REG_AUTOCALEMP                                                 \
  0x19                            ///< Auto-calibration back-EMF result register
#define DRV2605_REG_FEEDBACK 0x1A ///< Feedback control register
#define DRV2605_REG_CONTROL1 0x1B ///< Control1 Register
#define DRV2605_REG_CONTROL2 0x1C ///< Control2 Register
#define DRV2605_REG_CONTROL3 0x1D ///< Control3 Register
#define DRV2605_REG_CONTROL4 0x1E ///< Control4 Register
#define DRV2605_REG_VBAT 0x21     ///< Vbat voltage-monitor register
#define DRV2605_REG_LRARESON 0x22 ///< LRA resonance-period register

typedef struct{
    uint8_t instanced;
    I2C_HandleTypeDef *hi2c; 
} hapticInstance_t;

static hapticInstance_t hInstance = {0};

static HAL_StatusTypeDef _wr(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val) {
    return HAL_I2C_Mem_Write(hi2c, DRV2605_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             &val, 1, DRV2605_I2C_TIMEOUT_MS);
}
static HAL_StatusTypeDef _rd(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *val) {
    return HAL_I2C_Mem_Read(hi2c,  DRV2605_ADDR, reg, I2C_MEMADD_SIZE_8BIT,
                             val, 1, DRV2605_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef _soft_reset(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_MODE, 0x80); // reset
    HAL_Delay(2);
    return st;
}

uint8_t hapticSetIntensity(I2C_HandleTypeDef *hi2c, uint8_t rated, uint8_t clamp) {
    HAL_StatusTypeDef st = _wr(hi2c, DRV2605_REG_RATEDV, rated);
    if (st) return (st != HAL_OK);
    return (_wr(hi2c, DRV2605_REG_CLAMPV, clamp) != HAL_OK);
}

uint8_t hapticInit(I2C_HandleTypeDef *hi2c) {
    hInstance.hi2c = hi2c;
    hInstance.instanced = 1;

    HAL_StatusTypeDef st;

    // Reset y salir a modo INTTRIG
    st = _soft_reset(hi2c);                                  if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_MODE, DRV2605_MODE_INTTRIG);  if (st) return 1;

    // no real-time-playback
    st = _wr(hi2c, DRV2605_REG_RTPIN, 0x00); 	            if (st) return 1;

    // strong click + end sequence
    st = _wr(hi2c, DRV2605_REG_WAVESEQ1, 1); 	            if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_WAVESEQ2, 0); 	            if (st) return 1;

    // no overdrive
    st = _wr(hi2c, DRV2605_REG_OVERDRIVE, 0); 	            if (st) return 1;

    st = _wr(hi2c, DRV2605_REG_SUSTAINPOS, 0); 	            if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_SUSTAINNEG, 0); 	            if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_BREAK, 0); 	                if (st) return 1;
    st = _wr(hi2c, DRV2605_REG_AUDIOMAX, 0x64); 	        if (st) return 1;


    // ERM: FEEDBACK bit7=0 (ERM). 0x36 es un baseline seguro (ganancia/BEMF)
    st = _wr(hi2c, DRV2605_REG_FEEDBACK, 0x36);              if (st) return 1;

    // CONTROL3 = 0x20 → ERM open-loop (ideal para empezar)
    st = _wr(hi2c, DRV2605_REG_CONTROL3, 0x20);              if (st) return 1;

    return (st != HAL_OK);
}

uint8_t hapticSelectLibrary(uint8_t lib){
    if(hInstance.instanced){
        return(_wr(hInstance.hi2c, DRV2605_REG_LIBRARY, 0x01) != HAL_OK);
    }
    return 1;
}

uint8_t hapticGoSequence(){
    return(_wr(hInstance.hi2c, DRV2605_REG_GO, 1) != HAL_OK);
}

uint8_t hapticStopSequence(){
    return(_wr(hInstance.hi2c, DRV2605_REG_GO, 0) != HAL_OK);
}

uint8_t hapticSetWaveform(uint8_t slot, uint8_t w){
    return(_wr(hInstance.hi2c, DRV2605_REG_WAVESEQ1 + slot, w) != HAL_OK);
}

uint8_t hapticSetSequence(uint8_t *steps, uint8_t n){
    if(!steps || n > 8) return 1;
    uint8_t st = 0;
    for (int i = 0; i < 8; i++) {
        if (i < n) st += hapticSetWaveform(i, steps[i]);
        else st += hapticSetWaveform(i, 0);
    }
    return (st > 0);
}

/**
 * Assigns a waveform to the initial sequencer slot and plays it
 * @param id Number of effect (1 - 123)
 */
uint8_t hapticTrigger(uint8_t id) {
	uint8_t st = 0;
	st += hapticSetWaveform(0, id);
	st += hapticGoSequence();
	return (st > 0);
};

char EFFECTS[][50] = {
  "strong click 100%", "strong click 60%", "strong click 30%",
  "sharp click 100%", "sharp click 60%", "sharp click 30%",
  "soft bump 100%", "soft bump 60%", "soft bump 30%",
  "double click 100%", "double click 60%", "triple click 100%",
  "soft fuzz 60%", "strong buzz 100%",
  "750 ms alert 100%", "1000ms alert 100%",
  "strong click 1 100%", "strong click 2 80%", "strong click 3 60%", "strong click 4 30%",
  "medium click 1 100%", "medium click 2 80%", "medium click 3 60%",
  "sharp tick 1 100%", "sharp tick 2 80%", "sharp tick 3 60%",
  "short double click strong 1 100%", "short double click strong 2 80%", "short double click strong 3 60%", "short double click strong 4 30%",
  "short double click medium 1 100%", "short double click medium 2 80%", "short double click medium 3 60%",
  "short double sharp tick 1 100%", "short double sharp tick 2 80%", "short double sharp tick 3 60%",
  "long double sharp click strong 1 100%", "long double sharp click strong 2 80%", "long double sharp click strong 3 60%", "long double sharp click strong 4 30%",
  "long double sharp click medium 1 100%", "long double sharp click medium 2 80%", "long double sharp click medium 3 60%",
  "long double sharp tick 1 100%", "long double sharp tick 2 80%", "long double sharp tick 3 60%",
  "buzz 1 100%", "buzz 2 80%", "buzz 3 60%", "buzz 4 40%", "buzz 5 20%",
  "pulsing strong 1 100%", "pulsing strong 2 60%", "pulsing medium 1 100%", "pulsing medium 2 60%",
  "pulsing sharp 1 100%", "pulsing sharp 2 60%",
  "transition click 1 100%", "transition click 2 80%", "transition click 3 60%", "transition click 4 40%", "transition click 5 20%", "transition click 6 10%",
  "hum 1 100%", "hum 2 80%", "hum 3 60%", "hum 4 40%", "hum 5 20%", "hum 6 10%",
  "ramp down long smooth 1", "ramp down long smooth 2",
  "ramp down medium smooth 1", "ramp down medium smooth 2",
  "ramp down short smooth 1", "ramp down short smooth 2",
  "ramp down long sharp 1", "ramp down long sharp 2",
  "ramp down medium sharp 1", "ramp down medium sharp 2",
  "ramp down short sharp 1", "ramp down short sharp 2",
  "ramp up long smooth 1", "ramp up long smooth 2",
  "ramp up medium smooth 1", "ramp up medium smooth 2",
  "ramp up short smooth 1", "ramp up short smooth 2",
  "ramp up long sharp 1", "ramp up long sharp 2",
  "ramp up medium sharp 1", "ramp up medium sharp 2",
  "ramp up short sharp 1", "ramp up short sharp 2",
  "ramp down long smooth 1 half", "ramp down long smooth 2 half",
  "ramp down medium smooth 1 half", "ramp down medium smooth 2 half",
  "ramp down short smooth 1 half", "ramp down short smooth 2 half",
  "ramp down long sharp 1 half", "ramp down long sharp 2 half",
  "ramp down medium sharp 1 half", "ramp down medium sharp 2 half",
  "ramp down short sharp 1 half", "ramp down short sharp 2 half",
  "ramp up long smooth 1 half", "ramp up long smooth 2 half",
  "ramp up medium smooth 1 half", "ramp up medium smooth 2 half",
  "ramp up short smooth 1 half", "ramp up short smooth 2 half",
  "ramp up long sharp 1 half", "ramp up long sharp 2 half",
  "ramp up medium sharp 1 half", "ramp up medium sharp 2 half",
  "ramp up short sharp 1 half", "ramp up short sharp 2 half",
  "long buzz no stop",
  "smooth hum 1 50%", "smooth hum 2 40%", "smooth hum 3 30%", "smooth hum 4 20%", "smooth hum 5 10%",
};
