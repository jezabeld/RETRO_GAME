/**
 * @file ScreenDrv.c
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

/* === Headers files inclusions ================================================================ */

#include "ScreenDrv.h"
#include "string.h"
#include "stdlib.h"

/* === Macros definitions ====================================================================== */

#define ST_CMD_DELAY 0x80 ///< Special signifier for command lists indicating delay follows
#define MAX_DELAY_MS 500  ///< Maximum delay value in milliseconds
#define TFT_IS_160X128 1  ///< Display size identifier (160x128 pixels)
#define TFT_XSTART 0      ///< X coordinate offset for display window
#define TFT_YSTART 0      ///< Y coordinate offset for display window

// ST7735R Display Commands
#define CMD_NOP 0x00     ///< No Operation
#define CMD_SWRESET 0x01 ///< Software Reset
#define CMD_RDDID 0x04   ///< Read Display ID
#define CMD_RDDST 0x09   ///< Read Display Status

#define CMD_SLPIN 0x10  ///< Sleep In
#define CMD_SLPOUT 0x11 ///< Sleep Out
#define CMD_PTLON 0x12  ///< Partial Mode On
#define CMD_NORON 0x13  ///< Normal Display Mode On

#define CMD_INVOFF 0x20  ///< Display Inversion Off
#define CMD_INVON 0x21   ///< Display Inversion On
#define CMD_DISPOFF 0x28 ///< Display Off
#define CMD_DISPON 0x29  ///< Display On
#define CMD_CASET 0x2A   ///< Column Address Set
#define CMD_RASET 0x2B   ///< Row Address Set
#define CMD_RAMWR 0x2C   ///< Memory Write
#define CMD_RAMRD 0x2E   ///< Memory Read

#define CMD_PTLAR 0x30  ///< Partial Area
#define CMD_TEOFF 0x34  ///< Tearing Effect Line OFF
#define CMD_TEON 0x35   ///< Tearing Effect Line On
#define CMD_MADCTL 0x36 ///< Memory Data Access Control
#define CMD_COLMOD 0x3A ///< Interface Pixel Format

#define CMD_FRMCTR1 0xB1 ///< Frame Rate Control (Normal mode)
#define CMD_FRMCTR2 0xB2 ///< Frame Rate Control (Idle mode)
#define CMD_FRMCTR3 0xB3 ///< Frame Rate Control (Partial mode)
#define CMD_INVCTR 0xB4  ///< Display Inversion Control
#define CMD_DISSET5 0xB6 ///< Display Function Set

#define CMD_PWCTR1 0xC0 ///< Power Control 1
#define CMD_PWCTR2 0xC1 ///< Power Control 2
#define CMD_PWCTR3 0xC2 ///< Power Control 3
#define CMD_PWCTR4 0xC3 ///< Power Control 4
#define CMD_PWCTR5 0xC4 ///< Power Control 5
#define CMD_VMCTR1 0xC5 ///< VCOM Control 1

#define CMD_RDID1 0xDA ///< Read ID1
#define CMD_RDID2 0xDB ///< Read ID2
#define CMD_RDID3 0xDC ///< Read ID3
#define CMD_RDID4 0xDD ///< Read ID4

#define CMD_GMCTRP1 0xE0 ///< Gamma '+' polarity Correction Characteristics Setting
#define CMD_GMCTRN1 0xE1 ///< Gamma '-' polarity Correction Characteristics Setting

#define CMD_PWCTR6 0xFC ///< Power Control 6

// Display Configuration Parameters
#define FRMCTR_RTNA 0x01 ///< Frame rate control: RTNA (number of clocks per line)
#define FRMCTR_FPA 0x2C  ///< Frame rate control: Front Porch (wait before each line)
#define FRMCTR_BPA 0x2D  ///< Frame rate control: Back Porch (wait after each line)
#define INVCTR_LINE 0x07 ///< Inversion control: Line inversion for all modes

#define PWCTR_AVDD (5 << 5) ///< Power control: AVDD voltage level (5V)
#define PWCTR_VRH 0x02      ///< Power control: VRH voltage level (±4.6V)
#define PWCTR_MODEAUTO 0x84 ///< Power control: Auto mode selection

// Memory Access Control (MADCTL) parameters
#define MADCTL_MY 0x80  ///< Row Address Order (mirror Y)
#define MADCTL_MX 0x40  ///< Column Address Order (mirror X)
#define MADCTL_MV 0x20  ///< Row/Column Exchange
#define MADCTL_ML 0x10  ///< Vertical Refresh Order
#define MADCTL_RGB 0x00 ///< RGB color order
#define MADCTL_BGR 0x08 ///< BGR color order
#define MADCTL_MH 0x04  ///< Horizontal Refresh Order

#define COLMOD_16BIT 0x05 ///< Interface Pixel Format: 16-bit color (RGB565)

/* === Private data type declarations ========================================================== */

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

/**
 * @brief Software delay using CPU cycles (blocking)
 * @param ms Delay time in milliseconds
 * @note Approximate delay for 84MHz clock, adjust multiplier for different frequencies
 */
static void tftDelay(uint32_t ms);

/**
 * @brief Assert Chip Select pin (CS low)
 * @param tft Pointer to TFT handle
 */
static void tftSelect(tft_t *tft);

/**
 * @brief Perform hardware reset of the display
 * @param tft Pointer to TFT handle
 */
static void tftReset(tft_t *tft);

/**
 * @brief Send a single command to the display
 * @param tft Pointer to TFT handle
 * @param cmd Command byte to send
 */
static void tftWriteCommand(tft_t *tft, uint8_t cmd);

/**
 * @brief Send data bytes to the display
 * @param tft Pointer to TFT handle
 * @param buff Data buffer to transmit
 * @param buff_size Size of data buffer in bytes
 */
static void tftWriteData(tft_t *tft, uint8_t *buff, size_t buff_size);

/**
 * @brief Send a command with parameters to the display
 * @param tft Pointer to TFT handle
 * @param commandByte Command to send
 * @param dataBytes Parameter data to send
 * @param numDataBytes Number of parameter bytes
 */
static void tftSendCommand(tft_t *tft, uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes);

/**
 * @brief Read parameters from the display
 * @param tft Pointer to TFT handle
 * @param commandByte Command to send for reading
 * @param dataBytes Buffer to store received data
 * @param numDataBytes Number of bytes to read
 */
static void receiveParams(tft_t *tft, uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);

/**
 * @brief Set display rotation/orientation
 * @param tft Pointer to TFT handle
 */
static void tftSetRotation(tft_t *tft);

/**
 * @brief Execute a list of initialization commands
 * @param tft Pointer to TFT handle
 * @param addr Pointer to command list array
 */
static void tftExecuteCommandList(tft_t *tft, const uint8_t *addr);

/**
 * @brief Send data using DMA (non-blocking)
 * @param tft Pointer to TFT handle
 * @param buff Data buffer to transmit
 * @param buff_size Size of data buffer in bytes
 * @note CS pin must be released in DMA completion callback
 */
static void tftWriteDataDMA(tft_t *tft, const uint8_t *buff, uint32_t buff_size);

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/**
 * @brief ST7735R Display Initialization Command Sequence
 * 
 * Complete initialization sequence for ST7735R controller.
 * Format: [num_commands, cmd1, num_args, arg1, arg2, ..., cmd2, num_args, arg1, ...]
 * If MSB of num_args is set (ST_CMD_DELAY), a delay follows the arguments.
 */
// clang-format off
static const uint8_t initCmds[] = {
	21,                             // 21 commands in list:
	CMD_SWRESET,   ST_CMD_DELAY, 	//  1: Software reset, 0 args, w/delay
	  150,                          //     150 ms delay
	CMD_SLPOUT,    ST_CMD_DELAY, 	//  2: Out of sleep mode, 0 args, w/delay
	  255,                          //     500 ms delay (255 means max value)
	CMD_FRMCTR1, 3,              	//  3: Framerate ctrl - normal mode, 3 arg: RTNA, FPA, BPA
	  FRMCTR_RTNA, FRMCTR_FPA, 		//     Rate = fosc/((RTNA x 2 + 40) x (LINE + FPA + BPA))
	  FRMCTR_BPA,					//	   fosc = 625kHz
	CMD_FRMCTR2, 3,              	//  4: Framerate ctrl - idle mode, 3 args:
	  FRMCTR_RTNA, FRMCTR_FPA, 		//     Rate = fosc/((RTNA x 2 + 40) x (LINE + FPA + BPA))
	  FRMCTR_BPA,
	CMD_FRMCTR3, 6,              	//  5: Framerate - partial mode, 6 args:
	  FRMCTR_RTNA, FRMCTR_FPA, 		//     RTNA, FPA, BPA for Dot inversion mode
	  FRMCTR_BPA, FRMCTR_RTNA,  	//     RTNA, FPA, BPA for Line inversion mode
	  FRMCTR_FPA, FRMCTR_BPA,
	CMD_INVCTR,  1,              	//  6: Display inversion ctrl, 1 arg:
	  INVCTR_LINE,                  //     Line inversion for all modes (Normal mode, Idle mode, partial mode)
	CMD_PWCTR1,  3,              	//  7: Power control, 3 args, no delay:
	  PWCTR_AVDD|PWCTR_VRH,			//	   AVDD: 5V, VRHP: +4.6V
	  PWCTR_VRH,                    //     VRHN: -4.6V
	  PWCTR_MODEAUTO,               //     AUTO mode
	CMD_PWCTR2,  1,              	//  8: Power control, 1 arg, no delay:
	  0xC5,                        	//     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
	CMD_PWCTR3,  2,              	//  9: Power control, 2 args, no delay:
	  0x0A,                        	//     Opamp current small
	  0x00,                         //     Boost frequency
	CMD_PWCTR4,  2,              	// 10: Power control, 2 args, no delay:
	  0x8A,                         //     BCLK/2,
	  0x2A,                         //     opamp current small & medium low
	CMD_PWCTR5,  2,              	// 11: Power control, 2 args, no delay:
	  0x8A, 0xEE,
	CMD_VMCTR1,  1,              	// 12: Power control, 1 arg, no delay:
	  0x0E,
	CMD_INVOFF,  0,              	// 13: Don't invert display, no args
	CMD_MADCTL,  1,              	// 14: Mem access ctl (directions), 1 arg:
	  MADCTL_MY|MADCTL_MX|MADCTL_RGB,//     row/col addr, top-to-bottom refresh, RGB mode
	CMD_COLMOD,  1,              	// 15: set color mode, 1 arg, no delay:
	  COLMOD_16BIT,                 //     16-bit color
	CMD_CASET,   4,              	// 16: Column addr set, 4 args, no delay:
	  0x00, 0x00,                   //     XSTART = 0 (2 bytes data)
	  0x00, 0x7F,                   //     XEND = 127 (2 bytes data)
	CMD_RASET,   4,              	// 17: Row addr set, 4 args, no delay:
	  0x00, 0x00,                   //     XSTART = 0
	  0x00, 0x9F,                 	//     XEND = 159
	CMD_GMCTRP1, 16      ,       	// 18: Gamma Adjustments (pos. polarity), 16 args + delay:
	  0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
	  0x37, 0x32, 0x29, 0x2d,       //     accurate colors)
	  0x29, 0x25, 0x2B, 0x39,
	  0x00, 0x01, 0x03, 0x10,
	CMD_GMCTRN1, 16      ,       	// 19: Gamma Adjustments (neg. polarity), 16 args + delay:
	  0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
	  0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
	  0x2E, 0x2E, 0x37, 0x3F,
	  0x00, 0x00, 0x02, 0x10,
	CMD_NORON,     ST_CMD_DELAY, 	// 20: Normal display on, no args, w/delay
	  10,                           //     10 ms delay
	CMD_DISPON,    ST_CMD_DELAY, 	// 21: Display on, no args w/delay
	  100                         	//     100 ms delay
};
// clang-format on

/* === Private function implementation ========================================================= */

/**
 * @brief Software delay using CPU cycles (blocking)
 * @param ms Delay time in milliseconds
 * @note Approximate delay for 84MHz clock, adjust multiplier for different frequencies
 */
static void tftDelay(uint32_t ms) {
    volatile uint32_t count = ms * 21000; // Adjust multiplier based on CPU frequency
    while (count--) {
        __NOP(); // Prevent compiler optimization
    }
}

/* === Public function implementation ========================================================== */

void tftSelect(tft_t *tft) {
    HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_RESET);
}
void tftUnselect(tft_t *tft) {
    HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_SET);
}
void tftReset(tft_t *tft) {
    // Hardware reset sequence per ST7735R datasheet
    HAL_GPIO_WritePin(tft->resPort, tft->resPin, GPIO_PIN_RESET);
    tftDelay(10); // Hold reset for 10ms
    HAL_GPIO_WritePin(tft->resPort, tft->resPin, GPIO_PIN_SET);
    tftDelay(120); // Wait for controller stabilization
}

void tftWriteCommand(tft_t *tft, uint8_t cmd) {
    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_RESET); // Command mode (DC low)
    HAL_SPI_Transmit(tft->hSpi, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}
void tftWriteData(tft_t *tft, uint8_t *buff, size_t buff_size) {
    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET); // Data mode (DC high)
    HAL_SPI_Transmit(tft->hSpi, buff, buff_size, HAL_MAX_DELAY);
}

void tftSendCommand(tft_t *tft, uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes) {
    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit(tft->hSpi, &commandByte, 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET); // Data mode
    for (int i = 0; i < numDataBytes; i++) {
        HAL_SPI_Transmit(tft->hSpi, dataBytes, 1, HAL_MAX_DELAY);
        dataBytes++; // Advance pointer
    }
}

void receiveParams(tft_t *tft, uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes) {
    HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_RESET); // CS low
    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_RESET); // command mode

    HAL_SPI_Transmit(tft->hSpi, &commandByte, 1, HAL_MAX_DELAY); // send command

    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET); // param/data mode

    for (int i = 0; i < numDataBytes; i++) { // receive params
        HAL_SPI_Receive(tft->hSpi, dataBytes, 1, HAL_MAX_DELAY);
        dataBytes++; // avanza el puntero
    }
    HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_SET); // CS high
}

void tftSetRotation(tft_t *tft) {
    uint8_t madctl = MADCTL_MX | MADCTL_MY | MADCTL_RGB;
    tftSendCommand(tft, CMD_MADCTL, &madctl, 1);
}

/**
 * @brief Execute initialization command sequence from array
 * @param tft Pointer to TFT handle
 * @param addr Pointer to command array
 * @note Processes command list format with optional delays
 */
void tftExecuteCommandList(tft_t *tft, const uint8_t *addr) {
    uint8_t numCommands, cmd, numArgs;
    uint16_t ms;

    numCommands = *(addr++); // First byte is number of commands
    while (numCommands--) {
        cmd = *(addr++);
        numArgs = *(addr++);
        ms = numArgs & ST_CMD_DELAY; // Check if delay follows
        numArgs &= ~ST_CMD_DELAY;    // Mask out delay bit

        tftSendCommand(tft, cmd, addr, numArgs);
        addr += numArgs;

        if (ms) {
            ms = *(addr++);
            if (ms == 255) ms = MAX_DELAY_MS; // Special case for max delay
            tftDelay(ms);
        }
    }
}

uint8_t tftInit(tft_t *tft, SPI_HandleTypeDef *hSpi, GPIO_TypeDef *csPort, uint16_t csPin, GPIO_TypeDef *dcPort,
                uint16_t dcPin, GPIO_TypeDef *resPort, uint16_t resPin) {
    if (!tft || !hSpi) return 1; // Invalid parameters

    tft->hSpi = hSpi;
    tft->csPort = csPort;
    tft->csPin = csPin;
    tft->dcPort = dcPort;
    tft->dcPin = dcPin;
    tft->resPort = resPort;
    tft->resPin = resPin;

    tftSelect(tft);
    tftReset(tft);
    tftExecuteCommandList(tft, initCmds);
    tftUnselect(tft);

    return 0; // Success
}

void tftSetAddressWindow(tft_t *tft, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // Set column address range (CASET)
    tftWriteCommand(tft, (uint8_t) CMD_CASET);
    uint8_t data[] = { 0x00, x0 + TFT_XSTART, 0x00, x1 + TFT_XSTART };
    tftWriteData(tft, data, sizeof(data));

    // Set row address range (RASET)
    tftWriteCommand(tft, (uint8_t) CMD_RASET);
    data[1] = y0 + TFT_YSTART;
    data[3] = y1 + TFT_YSTART;
    tftWriteData(tft, data, sizeof(data));

    // Prepare for memory write
    tftWriteCommand(tft, (uint8_t) CMD_RAMWR);
}

void tftDrawPixel(tft_t *tft, uint8_t x, uint8_t y, uint16_t color) {
    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;

    tftSelect(tft);

    tftSetAddressWindow(tft, x, y, x + 1, y + 1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    tftWriteData(tft, data, sizeof(data));

    tftUnselect(tft);
}

void tftWriteChar(tft_t *tft, uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    tftSetAddressWindow(tft, x, y, x + font.width - 1, y + font.height - 1);

    for (i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++) {
            if ((b << j) & 0x8000) {
                uint8_t data[] = { color >> 8, color & 0xFF };
                tftWriteData(tft, data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                tftWriteData(tft, data, sizeof(data));
            }
        }
    }
}

void tftWriteString(tft_t *tft, uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color,
                    uint16_t bgcolor) {
    tftSelect(tft);

    while (*str) {
        if (x + font.width >= TFT_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= TFT_HEIGHT) {
                break;
            }

            if (*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        tftWriteChar(tft, x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    tftUnselect(tft);
}

void tftFillRectangle(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // Boundary clipping
    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;
    if ((x + w - 1) >= TFT_WIDTH) w = TFT_WIDTH - x;
    if ((y + h - 1) >= TFT_HEIGHT) h = TFT_HEIGHT - y;

    tftSelect(tft);
    tftSetAddressWindow(tft, x, y, x + w - 1, y + h - 1);

    // Optimize by preparing one line buffer and reusing it
    uint8_t pixel[] = { color >> 8, color & 0xFF };
    uint8_t *line = malloc(w * sizeof(pixel));
    for (x = 0; x < w; ++x) memcpy(line + x * sizeof(pixel), pixel, sizeof(pixel));

    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET);
    for (y = h; y > 0; y--) HAL_SPI_Transmit(tft->hSpi, line, w * sizeof(pixel), HAL_MAX_DELAY);

    free(line);
    tftUnselect(tft);
}

void tftFillScreen(tft_t *tft, uint16_t color) {
    tftFillRectangle(tft, 0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

void tftDrawImage(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {
    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;
    if ((x + w - 1) >= TFT_WIDTH) return;
    if ((y + h - 1) >= TFT_HEIGHT) return;

    tftSelect(tft);
    tftSetAddressWindow(tft, x, y, x + w - 1, y + h - 1);
    tftWriteData(tft, (uint8_t *) data, sizeof(uint16_t) * w * h);
    tftUnselect(tft);
}

/* ---------------------------- DMA Functions ---------------------------------------*/

/**
 * @brief Send data using DMA (non-blocking)
 * @param tft Pointer to TFT handle
 * @param buff Data buffer to transmit
 * @param buff_size Size of data buffer in bytes
 * @note CS pin must be released in DMA completion callback
 */
static void tftWriteDataDMA(tft_t *tft, const uint8_t *buff, uint32_t buff_size) {
    HAL_GPIO_WritePin(tft->dcPort, tft->dcPin, GPIO_PIN_SET);   // Data mode
    HAL_GPIO_WritePin(tft->csPort, tft->csPin, GPIO_PIN_RESET); // Keep CS low during transfer
    HAL_SPI_Transmit_DMA(tft->hSpi, (uint8_t *) buff, buff_size);
}

void tftDrawImageDMA(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {
    // Boundary checks
    if ((x >= TFT_WIDTH) || (y >= TFT_HEIGHT)) return;
    if ((x + w - 1) >= TFT_WIDTH) return;
    if ((y + h - 1) >= TFT_HEIGHT) return;

    tftSelect(tft);
    tftSetAddressWindow(tft, x, y, x + w - 1, y + h - 1);
    tftWriteDataDMA(tft, (const uint8_t *) data, sizeof(uint16_t) * w * h);
    // Note: CS pin must be released in DMA completion callback
}

#ifdef USE_DMA_CB
/**
 * @brief DMA Transfer Complete Callback (when USE_DMA_CB is defined)
 * @param hspi Pointer to SPI handle that completed the transfer
 * @note Overrides weak HAL implementation to release CS pin
 * @warning A global variable `tft_t tft` must be declared and initialized to use with this callback.
 */

extern tft_t tft;
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    tftUnselect(&tft); // Release CS pin after DMA transfer
}
#endif

/* === End of source code ====================================================================== */