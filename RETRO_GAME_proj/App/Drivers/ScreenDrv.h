/**
 * @file ScreenDrv.h
 * @brief ST7735R TFT LCD Driver for STM32F446xx
 * 
 * This driver provides an interface to control ST7735R-based TFT LCD displays
 * using SPI communication. It supports basic graphics operations, text rendering,
 * and DMA-accelerated image drawing.
 * 
 * @date May 5, 2025
 * @author jez
 * 
 * @note This driver is designed for 128x160 pixel displays (1.8" TFT)
 */

#ifndef DRIVERS_SCREENDRV_H_
#define DRIVERS_SCREENDRV_H_

/* === Headers files inclusions ================================================================ */

#include <stdint.h>
#include <stdbool.h>

#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include "fonts.h"

/* === Public macros definitions =============================================================== */

//#define USE_DMA_CB 1             ///< Enable DMA completion callback (uncomment to use)

#define TFT_WIDTH 128  ///< Display width in pixels
#define TFT_HEIGHT 160 ///< Display height in pixels (for 1.8" TFT)

#define TFT_COLOR_BLACK 0x0000   ///< Black color (RGB565)
#define TFT_COLOR_BLUE 0x001F    ///< Blue color (RGB565)
#define TFT_COLOR_RED 0xF800     ///< Red color (RGB565)
#define TFT_COLOR_GREEN 0x07E0   ///< Green color (RGB565)
#define TFT_COLOR_CYAN 0x07FF    ///< Cyan color (RGB565)
#define TFT_COLOR_MAGENTA 0xF81F ///< Magenta color (RGB565)
#define TFT_COLOR_YELLOW 0xFFE0  ///< Yellow color (RGB565)
#define TFT_COLOR_WHITE 0xFFFF   ///< White color (RGB565)

/* === Public data type declarations =========================================================== */

/**
 * @brief TFT display handle structure
 * 
 * This structure contains all the hardware interface information
 * needed to communicate with the ST7735R TFT display.
 */
typedef struct {
    SPI_HandleTypeDef *hSpi; ///< Pointer to SPI handle for display communication
    GPIO_TypeDef *csPort;    ///< GPIO port for Chip Select pin
    uint16_t csPin;          ///< Chip Select pin number
    GPIO_TypeDef *dcPort;    ///< GPIO port for Data/Command pin
    uint16_t dcPin;          ///< Data/Command pin number
    GPIO_TypeDef *resPort;   ///< GPIO port for Reset pin
    uint16_t resPin;         ///< Reset pin number
} tft_t;

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

/**
 * @brief Initialize the TFT display
 * 
 * Initializes the TFT display with the provided hardware configuration.
 * Performs hardware reset and sends initialization command sequence.
 * 
 * @param tft Pointer to TFT handle structure
 * @param hSpi Pointer to SPI handle for communication
 * @param csPort GPIO port for Chip Select pin
 * @param csPin Chip Select pin number
 * @param dcPort GPIO port for Data/Command pin  
 * @param dcPin Data/Command pin number
 * @param resPort GPIO port for Reset pin
 * @param resPin Reset pin number
 * @retval 0 Success
 * @retval 1 Error (invalid parameters)
 */
uint8_t tftInit(tft_t *tft, SPI_HandleTypeDef *hSpi, GPIO_TypeDef *csPort, uint16_t csPin, GPIO_TypeDef *dcPort,
                uint16_t dcPin, GPIO_TypeDef *resPort, uint16_t resPin);

/**
 * @brief Set the active drawing window on the display
 * @param tft Pointer to TFT handle
 * @param x0 Start X coordinate (0-127)
 * @param y0 Start Y coordinate (0-159)
 * @param x1 End X coordinate (0-127)
 * @param y1 End Y coordinate (0-159)
 * @warning No boundary validation is performed - coordinates are sent directly to hardware
 */
void tftSetAddressWindow(tft_t *tft, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

/**
 * @brief Draw a single pixel
 * @param tft Pointer to TFT handle
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-159)
 * @param color Pixel color in RGB565 format
 * @note Performs boundary validation - out-of-bounds coordinates are ignored
 */
void tftDrawPixel(tft_t *tft, uint8_t x, uint8_t y, uint16_t color);

/**
 * @brief Write a single character to the display
 * @param tft Pointer to TFT handle
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-159)
 * @param ch Character to write
 * @param font Font definition to use
 * @param color Text color in RGB565 format
 * @param bgcolor Background color in RGB565 format
 */
void tftWriteChar(tft_t *tft, uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);

/**
 * @brief Write a string to the display with automatic line wrapping
 * @param tft Pointer to TFT handle
 * @param x Starting X coordinate (0-127)
 * @param y Starting Y coordinate (0-159)
 * @param str Null-terminated string to write
 * @param font Font definition to use
 * @param color Text color in RGB565 format
 * @param bgcolor Background color in RGB565 format
 */
void tftWriteString(tft_t *tft, uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color,
                    uint16_t bgcolor);

/**
 * @brief Fill a rectangular area with solid color
 * @param tft Pointer to TFT handle
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner  
 * @param w Width of rectangle
 * @param h Height of rectangle
 * @param color Fill color in RGB565 format
 * @note Performs boundary validation with clipping - out-of-bounds areas are ignored or clipped
 */
void tftFillRectangle(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Fill the entire screen with a solid color
 * @param tft Pointer to TFT handle
 * @param color Fill color in RGB565 format
 */
void tftFillScreen(tft_t *tft, uint16_t color);

/**
 * @brief Draw an image to the display (blocking)
 * @param tft Pointer to TFT handle
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param data Pointer to image data in RGB565 format
 * @note Performs boundary validation - out-of-bounds images are ignored
 */
void tftDrawImage(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

/**
 * @brief Draw an image to the display using DMA (non-blocking)
 * @param tft Pointer to TFT handle
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Image width in pixels
 * @param h Image height in pixels
 * @param data Pointer to image data in RGB565 format
 * 
 * @note Performs boundary validation - out-of-bounds images are ignored
 * @note DMA Setup Requirements:
 *       1. Configure SPI with DMA TX enabled
 *       2. Uncomment `USE_DMA_CB` to enable automatic CS release (callback created automatically), OR
 *       3. Manually call `tftUnselect()` in your DMA completion callback
 *       4. If using manual approach, implement `HAL_SPI_TxCpltCallback()` in your code
 *
 * @warning When using `USE_DMA_CB`, a global variable `tft_t tft` must be declared and initialized
 *          The automatic callback function references this global instance to release the CS pin
 * 
 * @warning CS pin must be released after DMA transfer completes
 * 
 * @code
 * // Example DMA completion callback implementation:
 * void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
 *     if (hspi == &hspi1) {  // Your SPI handle
 *         tftUnselect(&your_tft_handle);
 *     }
 * }
 * @endcode
 */
void tftDrawImageDMA(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

/**
 * @brief Release the display Chip Select pin
 * @param tft Pointer to TFT handle
 * @note Typically used in DMA completion callbacks
 */
void tftUnselect(tft_t *tft);

/* === End of header =========================================================================== */
#endif /* DRIVERS_SCREENDRV_H_ */