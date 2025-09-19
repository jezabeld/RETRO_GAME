/**
 * @file test_ScreenDrv.c
 *
 * @date Sep 9, 2025
 * @author jez
 */

/* === Headers files inclusions ================================================================ */

#include "unity.h"
#include "ScreenDrv.h"

#include "mock_stm32f446xx.h"
#include "mock_stm32f4xx_hal.h"
#include "mock_stm32f4xx_hal_spi.h"
#include "mock_fonts.h"

/* === Global variables for DMA callback testing ============================================== */

// Global tft handle required by HAL_SPI_TxCpltCallback when USE_DMA_CB is defined
tft_t tft;

/* === Private function implementation ========================================================= */
void setUp() { }
void tearDown() { }

void test_tftInit_invalid_tft_pointer_returns_error(void) {
    // Arrange
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    // Act & Assert
    uint8_t result = tftInit(NULL, &mock_spi, &mock_gpio, 1, &mock_gpio, 2, &mock_gpio, 3);
    TEST_ASSERT_EQUAL_UINT8(1, result);
}

void test_tftInit_invalid_spi_pointer_returns_error(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    // Act & Assert
    uint8_t result = tftInit(&mock_tft, NULL, &mock_gpio, 1, &mock_gpio, 2, &mock_gpio, 3);
    TEST_ASSERT_EQUAL_UINT8(1, result);
}

void test_tftInit_valid_parameters_returns_success(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    // Mock HAL calls that will be made during initialization
    HAL_GPIO_WritePin_Ignore();
    HAL_SPI_Transmit_IgnoreAndReturn(HAL_OK);

    // Act
    uint8_t result = tftInit(&mock_tft, &mock_spi, &mock_gpio, 1, &mock_gpio, 2, &mock_gpio, 3);

    // Assert
    TEST_ASSERT_EQUAL_UINT8(0, result);
    TEST_ASSERT_EQUAL_PTR(&mock_spi, mock_tft.hSpi);
    TEST_ASSERT_EQUAL_PTR(&mock_gpio, mock_tft.csPort);
    TEST_ASSERT_EQUAL_UINT16(1, mock_tft.csPin);
    TEST_ASSERT_EQUAL_PTR(&mock_gpio, mock_tft.dcPort);
    TEST_ASSERT_EQUAL_UINT16(2, mock_tft.dcPin);
    TEST_ASSERT_EQUAL_PTR(&mock_gpio, mock_tft.resPort);
    TEST_ASSERT_EQUAL_UINT16(3, mock_tft.resPin);
}

void test_tftSetAddressWindow_sets_column_and_row_addresses(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;

    uint8_t x0 = 10, y0 = 20, x1 = 50, y1 = 80;

    uint8_t cmd_caset = 0x2A;
    uint8_t cmd_raset = 0x2B;
    uint8_t cmd_ramwr = 0x2C;
    uint8_t dataX[] = { 0x00, x0, 0x00, x1 };
    uint8_t dataY[] = { 0x00, y0, 0x00, y1 };

    // Expect HAL calls for CASET command
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_caset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataX, 4, HAL_MAX_DELAY, HAL_OK);

    // Expect HAL calls for RASET command
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_raset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataY, 4, HAL_MAX_DELAY, HAL_OK);

    // Expect HAL calls for RAMWR command
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_ramwr, 1, HAL_MAX_DELAY, HAL_OK);

    // Act
    tftSetAddressWindow(&mock_tft, x0, y0, x1, y1);

    // Assert - CMock will verify all expected calls were made
}

void test_tftDrawPixel_valid_coordinates_draws_pixel(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    uint8_t x = 50, y = 75;
    uint16_t color = 0xF800; // Red

    // Expected behavior: function should execute all drawing operations
    uint8_t cmd_caset = 0x2A;
    uint8_t cmd_raset = 0x2B;
    uint8_t cmd_ramwr = 0x2C;
    uint8_t dataX[] = { 0x00, x, 0x00, x + 1 };
    uint8_t dataY[] = { 0x00, y, 0x00, y + 1 };
    uint8_t dataP[] = { color >> 8, color & 0xFF };

    // Select
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_RESET); // CS low

    // Expect HAL calls for CASET command
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_caset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataX, 4, HAL_MAX_DELAY, HAL_OK);

    // Expect HAL calls for RASET command
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_raset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataY, 4, HAL_MAX_DELAY, HAL_OK);

    // Expect HAL calls for RAMWR command
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // Command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_ramwr, 1, HAL_MAX_DELAY, HAL_OK);

    // Expect HAL calls for pixel send
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataP, 2, HAL_MAX_DELAY, HAL_OK);

    // Unselect
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_SET); // CS high

    // Act
    tftDrawPixel(&mock_tft, x, y, color);

    // Assert - Function completed successfully (boundary check passed)
}

void test_tftDrawPixel_x_coordinate_out_of_bounds_does_nothing(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    uint8_t x = 128; // Out of bounds (TFT_WIDTH = 128, so valid range is 0-127)
    uint8_t y = 50;
    uint16_t color = 0xF800;

    // Expect no HAL calls since function should return early

    // Act
    tftDrawPixel(&mock_tft, x, y, color);

    // Assert - No calls should be made, test passes if no unexpected calls
}

void test_tftDrawPixel_y_coordinate_out_of_bounds_does_nothing(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    uint8_t x = 50;
    uint8_t y = 160; // Out of bounds (TFT_HEIGHT = 160, so valid range is 0-159)
    uint16_t color = 0xF800;

    // Expect no HAL calls since function should return early

    // Act
    tftDrawPixel(&mock_tft, x, y, color);

    // Assert - No calls should be made, test passes if no unexpected calls
}

void test_tftFillRectangle_x_out_of_bounds_does_nothing(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    uint16_t x = 128; // Out of bounds (TFT_WIDTH = 128, so valid range is 0-127)
    uint16_t y = 50;
    uint16_t w = 20, h = 30;
    uint16_t color = 0xF800;

    // Expect no HAL calls since function should return early

    // Act
    tftFillRectangle(&mock_tft, x, y, w, h, color);

    // Assert - No calls should be made, test passes if no unexpected calls
}

void test_tftFillRectangle_y_out_of_bounds_does_nothing(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    uint16_t x = 50;
    uint16_t y = 160; // Out of bounds (TFT_HEIGHT = 160, so valid range is 0-159)
    uint16_t w = 20, h = 30;
    uint16_t color = 0xF800;

    // Expect no HAL calls since function should return early

    // Act
    tftFillRectangle(&mock_tft, x, y, w, h, color);

    // Assert - No calls should be made, test passes if no unexpected calls
}

void test_tftFillRectangle_valid_coordinates_fills_area(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    uint16_t x = 10, y = 20;
    uint16_t w = 128 - x;
    uint16_t h = 160 - y;
    uint16_t color = 0x07E0; // Green

    // Expected sequence: tftSelect -> tftSetAddressWindow -> fill data -> tftUnselect

    // tftSelect: CS pin LOW
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_RESET);

    // tftSetAddressWindow sequence (CASET, RASET, RAMWR commands)
    uint8_t cmd_caset = 0x2A;
    uint8_t cmd_raset = 0x2B;
    uint8_t cmd_ramwr = 0x2C;
    uint8_t dataX[] = { 0x00, x, 0x00, x + w - 1 }; // (10, 59)
    uint8_t dataY[] = { 0x00, y, 0x00, y + h - 1 }; // (20, 49)

    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // CASET command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_caset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // CASET data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataX, 4, HAL_MAX_DELAY, HAL_OK);

    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // RASET command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_raset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // RASET data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataY, 4, HAL_MAX_DELAY, HAL_OK);

    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // RAMWR command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_ramwr, 1, HAL_MAX_DELAY, HAL_OK);

    // Fill data: DC pin set to data mode + h lines of w*2 bytes each
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    for (int i = 0; i < h; i++) {
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, NULL, w * 2, HAL_MAX_DELAY, HAL_OK);
        HAL_SPI_Transmit_IgnoreArg_pData();
    }

    // tftUnselect: CS pin HIGH
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_SET);

    // Act
    tftFillRectangle(&mock_tft, x, y, w, h, color);

    // Assert - CMock will verify all expected calls were made in correct order
}

void test_tftFillRectangle_boundary_clipping_clips_width_and_height(void) {
    // Arrange
    tft_t mock_tft = { 0 };
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    mock_tft.hSpi = &mock_spi;
    mock_tft.dcPort = &mock_gpio;
    mock_tft.dcPin = 1;
    mock_tft.csPort = &mock_gpio;
    mock_tft.csPin = 2;

    // Rectangle that exceeds both width and height boundaries
    uint16_t x = 100, y = 140; // Start positions
    uint16_t w = 50, h = 50;   // Original dimensions (would exceed boundaries)
    uint16_t color = 0x001F;   // Blue

    // Expected clipped dimensions:
    // w_clipped = TFT_WIDTH - x = 128 - 100 = 28
    // h_clipped = TFT_HEIGHT - y = 160 - 140 = 20
    uint16_t w_clipped = 28;
    uint16_t h_clipped = 20;

    // Expected sequence: tftSelect -> tftSetAddressWindow -> fill data -> tftUnselect

    // tftSelect: CS pin LOW
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_RESET);

    // tftSetAddressWindow should be called with clipped dimensions
    uint8_t cmd_caset = 0x2A;
    uint8_t cmd_raset = 0x2B;
    uint8_t cmd_ramwr = 0x2C;
    uint8_t dataX[] = { 0x00, x, 0x00, x + w_clipped - 1 }; // (100, 127)
    uint8_t dataY[] = { 0x00, y, 0x00, y + h_clipped - 1 }; // (140, 159)

    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // CASET command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_caset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // CASET data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataX, 4, HAL_MAX_DELAY, HAL_OK);

    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // RASET command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_raset, 1, HAL_MAX_DELAY, HAL_OK);
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // RASET data mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataY, 4, HAL_MAX_DELAY, HAL_OK);

    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // RAMWR command mode
    HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_ramwr, 1, HAL_MAX_DELAY, HAL_OK);

    // Fill data with clipped dimensions: h_clipped lines of w_clipped*2 bytes each
    HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
    for (int i = 0; i < h_clipped; i++) {
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, NULL, w_clipped * 2, HAL_MAX_DELAY, HAL_OK);
        HAL_SPI_Transmit_IgnoreArg_pData();
    }

    // tftUnselect: CS pin HIGH
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_SET);

    // Act
    tftFillRectangle(&mock_tft, x, y, w, h, color);

    // Assert - CMock will verify clipped dimensions are used correctly
}

// Function pointer type for draw image functions
typedef void (*draw_image_func_t)(tft_t *tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);

void test_draw_image_x_out_of_bounds_does_nothing(void) {
    const uint16_t test_data[] = { 0x1234, 0x5678 };

    // Test both tftDrawImage and tftDrawImageDMA
    draw_image_func_t functions[] = { tftDrawImage, tftDrawImageDMA };
    const char *function_names[] = { "tftDrawImage", "tftDrawImageDMA" };

    for (int func_idx = 0; func_idx < 2; func_idx++) {
        // Arrange
        tft_t mock_tft = { 0 };
        SPI_HandleTypeDef mock_spi = { 0 };
        GPIO_TypeDef mock_gpio = { 0 };

        mock_tft.hSpi = &mock_spi;
        mock_tft.dcPort = &mock_gpio;
        mock_tft.dcPin = 1;
        mock_tft.csPort = &mock_gpio;
        mock_tft.csPin = 2;

        uint16_t x = 128; // Out of bounds
        uint16_t y = 50, w = 20, h = 30;

        // Expect no HAL calls since function should return early

        // Act
        functions[func_idx](&mock_tft, x, y, w, h, test_data);

        // Assert - No calls should be made, test passes if no unexpected calls
    }
}

void test_draw_image_y_out_of_bounds_does_nothing(void) {
    const uint16_t test_data[] = { 0x1234, 0x5678 };

    // Test both tftDrawImage and tftDrawImageDMA
    draw_image_func_t functions[] = { tftDrawImage, tftDrawImageDMA };
    const char *function_names[] = { "tftDrawImage", "tftDrawImageDMA" };

    for (int func_idx = 0; func_idx < 2; func_idx++) {
        // Arrange
        tft_t mock_tft = { 0 };
        SPI_HandleTypeDef mock_spi = { 0 };
        GPIO_TypeDef mock_gpio = { 0 };

        mock_tft.hSpi = &mock_spi;
        mock_tft.dcPort = &mock_gpio;
        mock_tft.dcPin = 1;
        mock_tft.csPort = &mock_gpio;
        mock_tft.csPin = 2;

        uint16_t x = 50;
        uint16_t y = 160; // Out of bounds
        uint16_t w = 20, h = 30;

        // Expect no HAL calls since function should return early

        // Act
        functions[func_idx](&mock_tft, x, y, w, h, test_data);

        // Assert - No calls should be made, test passes if no unexpected calls
    }
}

void test_draw_image_width_exceeds_boundary_does_nothing(void) {
    const uint16_t test_data[] = { 0x1234, 0x5678 };

    // Test both tftDrawImage and tftDrawImageDMA
    draw_image_func_t functions[] = { tftDrawImage, tftDrawImageDMA };
    const char *function_names[] = { "tftDrawImage", "tftDrawImageDMA" };

    for (int func_idx = 0; func_idx < 2; func_idx++) {
        // Arrange
        tft_t mock_tft = { 0 };
        SPI_HandleTypeDef mock_spi = { 0 };
        GPIO_TypeDef mock_gpio = { 0 };

        mock_tft.hSpi = &mock_spi;
        mock_tft.dcPort = &mock_gpio;
        mock_tft.dcPin = 1;
        mock_tft.csPort = &mock_gpio;
        mock_tft.csPin = 2;

        uint16_t x = 100, y = 50;
        uint16_t w = 50, h = 30; // x + w - 1 = 149 >= 128

        // Expect no HAL calls since function should return early

        // Act
        functions[func_idx](&mock_tft, x, y, w, h, test_data);

        // Assert - No calls should be made, test passes if no unexpected calls
    }
}

void test_draw_image_height_exceeds_boundary_does_nothing(void) {
    const uint16_t test_data[] = { 0x1234, 0x5678 };

    // Test both tftDrawImage and tftDrawImageDMA
    draw_image_func_t functions[] = { tftDrawImage, tftDrawImageDMA };
    const char *function_names[] = { "tftDrawImage", "tftDrawImageDMA" };

    for (int func_idx = 0; func_idx < 2; func_idx++) {
        // Arrange
        tft_t mock_tft = { 0 };
        SPI_HandleTypeDef mock_spi = { 0 };
        GPIO_TypeDef mock_gpio = { 0 };

        mock_tft.hSpi = &mock_spi;
        mock_tft.dcPort = &mock_gpio;
        mock_tft.dcPin = 1;
        mock_tft.csPort = &mock_gpio;
        mock_tft.csPin = 2;

        uint16_t x = 50, y = 140;
        uint16_t w = 30, h = 50; // y + h - 1 = 189 >= 160

        // Expect no HAL calls since function should return early

        // Act
        functions[func_idx](&mock_tft, x, y, w, h, test_data);

        // Assert - No calls should be made, test passes if no unexpected calls
    }
}

void test_draw_image_valid_coordinates_draws_image(void) {
    const uint16_t test_data[] = { 0xF800, 0x07E0, 0x001F, 0xFFFF }; // R, G, B, White

    // Test both tftDrawImage and tftDrawImageDMA
    draw_image_func_t functions[] = { tftDrawImage, tftDrawImageDMA };
    const char *function_names[] = { "tftDrawImage", "tftDrawImageDMA" };

    for (int func_idx = 0; func_idx < 2; func_idx++) {
        // Arrange
        tft_t mock_tft = { 0 };
        SPI_HandleTypeDef mock_spi = { 0 };
        GPIO_TypeDef mock_gpio = { 0 };

        mock_tft.hSpi = &mock_spi;
        mock_tft.dcPort = &mock_gpio;
        mock_tft.dcPin = 1;
        mock_tft.csPort = &mock_gpio;
        mock_tft.csPin = 2;

        uint16_t x = 10, y = 20, w = 50, h = 30;

        // Expected sequence: tftSelect -> tftSetAddressWindow -> write data -> [tftUnselect for normal, not for DMA]

        // tftSelect: CS pin LOW
        HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_RESET);

        // tftSetAddressWindow sequence (CASET, RASET, RAMWR commands)
        uint8_t cmd_caset = 0x2A;
        uint8_t cmd_raset = 0x2B;
        uint8_t cmd_ramwr = 0x2C;
        uint8_t dataX[] = { 0x00, x, 0x00, x + w - 1 }; // (10, 59)
        uint8_t dataY[] = { 0x00, y, 0x00, y + h - 1 }; // (20, 49)

        HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // CASET command mode
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_caset, 1, HAL_MAX_DELAY, HAL_OK);
        HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // CASET data mode
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataX, 4, HAL_MAX_DELAY, HAL_OK);

        HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // RASET command mode
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_raset, 1, HAL_MAX_DELAY, HAL_OK);
        HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // RASET data mode
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, dataY, 4, HAL_MAX_DELAY, HAL_OK);

        HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_RESET); // RAMWR command mode
        HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, &cmd_ramwr, 1, HAL_MAX_DELAY, HAL_OK);

        // Image data transmission - different for each function
        uint32_t data_size = sizeof(uint16_t) * w * h;

        if (func_idx == 0) {
            // tftDrawImage: uses tftWriteData (blocking SPI) + tftUnselect
            HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET); // Data mode
            HAL_SPI_Transmit_ExpectAndReturn(&mock_spi, (uint8_t *) test_data, data_size, HAL_MAX_DELAY, HAL_OK);

            // tftUnselect: CS pin HIGH
            HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_SET);
        } else {
            // tftDrawImageDMA: uses tftWriteDataDMA (non-blocking SPI) + NO tftUnselect
            HAL_GPIO_WritePin_Expect(&mock_gpio, 1, GPIO_PIN_SET);   // Data mode
            HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_RESET); // Keep CS low during DMA
            HAL_SPI_Transmit_DMA_ExpectAndReturn(&mock_spi, (uint8_t *) test_data, data_size, HAL_OK);

            // NO tftUnselect expected - CS should be released in DMA callback
        }

        // Act
        functions[func_idx](&mock_tft, x, y, w, h, test_data);

        // Assert - CMock will verify expected calls were made
    }
}

void test_HAL_SPI_TxCpltCallback_releases_CS_pin(void) {
    // Arrange
    SPI_HandleTypeDef mock_spi = { 0 };
    GPIO_TypeDef mock_gpio = { 0 };

    // Configure the global tft handle used by the callback
    tft.csPort = &mock_gpio;
    tft.csPin = 2;

    // Expect tftUnselect behavior: CS pin HIGH
    HAL_GPIO_WritePin_Expect(&mock_gpio, 2, GPIO_PIN_SET);

    // Act - Call the DMA completion callback
    HAL_SPI_TxCpltCallback(&mock_spi);

    // Assert - CMock will verify CS pin was set HIGH
}

/* === End of source code ====================================================================== */