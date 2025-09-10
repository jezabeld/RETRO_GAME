/**
 * @file fonts.h
 * @brief Font definitions for TFT display text rendering
 * 
 * This module provides bitmap font definitions for use with the TFT display driver.
 * Includes multiple font sizes with corresponding character data.
 * 
 * @date Jun 19, 2025
 * @author jez
 * 
 * @note All fonts use monospace bitmap format with fixed character width
 */

#ifndef SCREENDRV_INC_FONTS_H_
#define SCREENDRV_INC_FONTS_H_

/* === Headers files inclusions ================================================================ */

#include <stdint.h>

/* === Public data type declarations =========================================================== */

/**
 * @brief Font definition structure
 * 
 * Contains all information needed to render a bitmap font including
 * character dimensions and pointer to bitmap data.
 */
typedef struct {
    const uint8_t width;     ///< Character width in pixels
    uint8_t height;          ///< Character height in pixels
    const uint16_t *data;    ///< Pointer to bitmap font data array
} FontDef;

/* === Public variable declarations ============================================================ */

/**
 * @brief Small font (7x10 pixels)
 */
extern FontDef Font_7x10;

/**
 * @brief Medium font (11x18 pixels)
 */
extern FontDef Font_11x18;

/**
 * @brief Large font (16x26 pixels)
 */
extern FontDef Font_16x26;

/* === End of header =========================================================================== */

#endif /* SCREENDRV_INC_FONTS_H_ */
