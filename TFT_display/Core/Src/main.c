/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "API_uart.h"
#include "TFT_ST7735.h"
#include "testimg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
//static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define CMD_RDID1 0xDA
#define CMD_RDID2 0xDB
#define CMD_RDID3 0xDC
#define CMD_RDID4 0xDD
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  //MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  uartInit();
  //spiInit(1, 2, 0, 0);

  uartSendString("Initializing...\r\n");

  /*
  const uint8_t CMD_SWRESET	= 0x01;
  const uint8_t CMD_SLPOUT 	= 0x11;
  const uint8_t CMD_RDDID 	= 0x04;
*/

  tft_t myTft;

  tftInit(&myTft, &hspi1, TFT_CS_GPIO_Port, TFT_CS_Pin, TFT_DC_GPIO_Port, TFT_DC_Pin, TFT_RS_GPIO_Port, TFT_RS_Pin);

  uint8_t cmd;

  uint8_t  id_buf[1];
  cmd = CMD_RDID1;
  receiveParams(&myTft, cmd, id_buf, 2u);
  cmd = CMD_RDID2;
  receiveParams(&myTft, cmd, &id_buf[2], 1u);
  cmd = CMD_RDID3;
  receiveParams(&myTft, cmd, &id_buf[3], 1u);

  // Ahora:
  // id_buf[1] = Manufacturer ID
  // id_buf[2] = Version ID
  // id_buf[3] = Driver ID

  uartSendString("Manufacturer ID: ");
  uartSendValue(id_buf[1]);
  uartSendString(", Version ID: ");
  uartSendValue(id_buf[2]);
  uartSendString(", Driver ID: ");
  uartSendValue(id_buf[3]);
  uartSendString("\r\n");


  // ---------- Comenzar escritura de memoria ----------
//  tftSetAddressWindow(myTft, 0,0, 127,159);
  //uint16_t color = 0xF800; // Rojo RGB565
  tftFillScreen(&myTft, TFT_COLOR_RED);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  // Check border
	  tftFillScreen(&myTft, TFT_COLOR_BLACK);

	  for(int x = 0; x < TFT_WIDTH; x++) {
		  tftDrawPixel(&myTft, x, 0, TFT_COLOR_RED);
		  tftDrawPixel(&myTft, x, TFT_HEIGHT-1, TFT_COLOR_RED);
	  }

	  for(int y = 0; y < TFT_HEIGHT; y++) {
		  tftDrawPixel(&myTft, 0, y, TFT_COLOR_RED);
		  tftDrawPixel(&myTft, TFT_WIDTH-1, y, TFT_COLOR_RED);
	  }

	  HAL_Delay(3000);

	  // Check fonts
	  tftFillScreen(&myTft, TFT_COLOR_BLACK);
	  tftWriteString(&myTft, 0, 0, "Font_7x10, red on black, lorem ipsum dolor sit amet", Font_7x10, TFT_COLOR_RED, TFT_COLOR_BLACK);
	  tftWriteString(&myTft, 0, 3*10, "Font_11x18, green, lorem ipsum", Font_11x18, TFT_COLOR_GREEN, TFT_COLOR_BLACK);
	  tftWriteString(&myTft, 0, 3*10+3*18, "Font_16x26", Font_16x26, TFT_COLOR_BLUE, TFT_COLOR_BLACK);
	  HAL_Delay(2000);

	  // Check colors
	  tftFillScreen(&myTft, TFT_COLOR_BLACK);
	  tftWriteString(&myTft, 0, 0, "BLACK", Font_11x18, TFT_COLOR_WHITE, TFT_COLOR_BLACK);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_BLUE);
	  tftWriteString(&myTft, 0, 0, "BLUE", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_BLUE);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_RED);
	  tftWriteString(&myTft, 0, 0, "RED", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_RED);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_GREEN);
	  tftWriteString(&myTft, 0, 0, "GREEN", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_GREEN);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_CYAN);
	  tftWriteString(&myTft, 0, 0, "CYAN", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_CYAN);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_MAGENTA);
	  tftWriteString(&myTft, 0, 0, "MAGENTA", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_MAGENTA);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_YELLOW);
	  tftWriteString(&myTft, 0, 0, "YELLOW", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_YELLOW);
	  HAL_Delay(500);

	  tftFillScreen(&myTft, TFT_COLOR_WHITE);
	  tftWriteString(&myTft, 0, 0, "WHITE", Font_11x18, TFT_COLOR_BLACK, TFT_COLOR_WHITE);
	  HAL_Delay(500);

	  // Display test image 128x128
	  tftDrawImage(&myTft, 0, 0, TFT_WIDTH, TFT_HEIGHT, (uint16_t*)image_128x160);
	  HAL_Delay(5000);

	  tftDrawImage(&myTft, 0, 0, TFT_WIDTH, TFT_HEIGHT, (uint16_t*)image_cockpit_desert_128x160);
	  HAL_Delay(5000);

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_RS_GPIO_Port, TFT_RS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_DC_Pin */
  GPIO_InitStruct.Pin = TFT_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_RS_Pin */
  GPIO_InitStruct.Pin = TFT_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_RS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_CS_Pin */
  GPIO_InitStruct.Pin = TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
