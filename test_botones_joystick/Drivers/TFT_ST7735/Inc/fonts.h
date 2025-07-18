/*
 * fonts.h
 *
 *  Created on: Jun 19, 2025
 *      Author: jez
 */

#ifndef TFT_ST7735_INC_FONTS_H_
#define TFT_ST7735_INC_FONTS_H_

/* vim: set ai et ts=4 sw=4: */
#include <stdint.h>

typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;


extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;

#endif /* TFT_ST7735_INC_FONTS_H_ */
