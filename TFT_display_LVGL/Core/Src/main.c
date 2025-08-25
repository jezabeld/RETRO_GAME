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
//#include "testimg.h"
#include "lv_port_disp.h" /* <- your display driver registration function   */
#include "lv_port_indev.h" /* <- your input device driver registration function */
#include "lvgl.h" /* <- brings in lv_init(), lv_tick_inc(), etc.    */
#include "test_icon.h"

//#include "demos/benchmark/lv_demo_benchmark.h"
#include "ui.h"

#include "AudioDrv.h"
/* ------------- USING LVGL v8.3.11 -------------------- */

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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim6;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
// reemplazo el init del DAC por estos dos:

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim); // calback devil de HAL redefinido


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


// macros para upruebas audio
//#define AUD_CASO_A
//#define AUD_CASO_B
//#define AUD_CASO_C
/* === Parámetros de audio === */
//#define FS_HZ        16000U     // tasa de muestreo para demos B/C
//#define BEEP_HZ      1000U      // frecuencia del beep para demo A
//#define DAC_MIN      800        // valores DAC para el beep cuadrado (A)
//#define DAC_MAX      3200
///* Wavetable simple (seno 256 muestras, 12-bit centrado en 2048) */
//#define WT_LEN 256
//uint16_t wavetable[WT_LEN];
///* Estado para ISR (A/B) */
//volatile uint32_t wt_idx = 0;
//volatile uint8_t  sq_toggle = 0;

tft_t myTft;

volatile uint16_t joy_raw[2] = { 0 }; /* [0]=PA1 X , [1]=PA4 Y        */

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
  MX_DMA_Init();
//  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
    uartInit();
    // spiInit(1, 2, 0, 0);


    uartSendString("Initializing...\r\n");

    lv_init();
    lv_log_register_print_cb(my_log_cb); /* ver logs */

    lv_port_disp_init(); /* <-- registra display */
    lv_port_indev_init(); /* <-- registra input devices */

    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joy_raw, 2);
//    audioInit();
//         ui_init();


    // Pruebas Juego
    /*--------------------------------------------------------------------
     * 1.  A dedicated screen for the in-game view
     *------------------------------------------------------------------*/
    static lv_obj_t* scr_game = NULL;
    static lv_obj_t* img_cockpit = NULL; /* foreground         */
    static lv_obj_t* cont_horizon = NULL; /* sky & mountains    */
    static lv_obj_t* rect_ground = NULL; /* moving ground strip*/

    /* resources that live in FLASH (const) */
    LV_IMG_DECLARE(cockpit128x160transp); /* generated PNG */

    static const lv_color_t sky_color = LV_COLOR_MAKE(0x74, 0x9c, 0xdb);
    static const lv_color_t ground_col = LV_COLOR_MAKE(0x46, 0x33, 0x21);

    /*--------------------------------------------------------------------
     * 2.  Screen builder
     *------------------------------------------------------------------*/
    void game_screen_create(void) {
        /* Create the screen once */
        scr_game = lv_obj_create(NULL);
        lv_obj_clear_flag(scr_game, LV_OBJ_FLAG_SCROLLABLE);

        /* ---------- Layer 0 : distant horizon (simple solid colour) */
        cont_horizon = lv_obj_create(scr_game);
        lv_obj_remove_style_all(cont_horizon); /* keep it light   */
        lv_obj_set_size(cont_horizon, 128, 160); /* full screen     */
        lv_obj_set_style_bg_opa(cont_horizon, LV_OPA_COVER, 0); /* enable background */
        lv_obj_set_style_bg_color(cont_horizon, sky_color, 0); /* sky background  */

        /* If you later want mountains/clouds:
           lv_obj_set_style_bg_img_src(cont_horizon, &mountains_img, 0);
        */

        /* ---------- Layer 1 : ground strip (variable height) */
        rect_ground = lv_obj_create(cont_horizon); /* child of sky    */
        lv_obj_remove_style_all(rect_ground);
        lv_obj_set_width(rect_ground, 128);
        lv_obj_set_align(rect_ground, LV_ALIGN_BOTTOM_MID);
        lv_obj_set_style_bg_opa(rect_ground, LV_OPA_COVER, 0); /* enable background */
        lv_obj_set_style_bg_color(rect_ground, ground_col, 0);

        /* start with 1/3 of the screen */
        lv_obj_set_height(rect_ground, 160 / 3);

        /* ---------- Layer 2 : cockpit overlay (with alpha) */
        img_cockpit = lv_img_create(scr_game);
        lv_img_set_src(img_cockpit, &cockpit128x160transp);
        lv_obj_align(img_cockpit, LV_ALIGN_LEFT_MID, 0, 0); /* fine-tune X if needed */
        lv_obj_clear_flag(img_cockpit, LV_OBJ_FLAG_CLICKABLE); /* purely decorative     */

        /* ---------- Focus group (do NOT include cockpit) */
        lv_group_t* g = lv_port_indev_get_group();
        if (g) {
            lv_group_remove_all_objs(g);
            /* add in-game UI widgets here if you have any */
        }

        lv_scr_load(scr_game); /* make it the active screen */
    }

    /*--------------------------------------------------------------------
     * 3.  Simple “camera” update – call every frame or timer
     *------------------------------------------------------------------*/
    void game_view_update(void /*-100 … +100*/) {
        /* Map pitch to a ground height: 0 % ⇒ horizon at mid-screen
           +100 ⇒ climb (ground low) ; -100 ⇒ dive (ground high)      */
        uint16_t h = lv_map(joy_raw[1], 0, 4095, 160, 0); /* pixels   */
        lv_obj_set_height(rect_ground, h);
    }

    /*--------------------------------------------------------------------
     * 4.  Example usage from your main loop / LVGL timer
     *------------------------------------------------------------------*/
    /* somewhere after initializing LVGL, when you enter the game: */
    game_screen_create();

         /* Change Active Screen's background color */
//         lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN);
//         lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
//
//         /* Create a spinner */
//         lv_obj_t * spinner = lv_spinner_create(lv_scr_act(), 1000, 60);
//         lv_obj_set_size(spinner, 64, 64);
//         lv_obj_align(spinner, LV_ALIGN_BOTTOM_MID, 0, 0);
//
//         /* Ahora sí, creamos la etiqueta */
//         /* icono */
//         lv_obj_t * img = lv_img_create(lv_scr_act());
//         lv_img_set_src(img, &test_icon_16x16);
//         lv_obj_center(img);                 /* centrado */
//
//         /* texto */
//         lv_obj_t * lab = lv_label_create(lv_scr_act());
//         lv_label_set_text(lab, "Hola LVGL");
//    //     lv_obj_align(lab, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
//         lv_obj_align_to(lab, img, LV_ALIGN_OUT_TOP_MID, 0, -4);/* 4 px encima del icono */

    //   lv_demo_benchmark();
//    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
//      HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
//      HAL_TIM_Base_Stop(&htim6);
//      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
        lv_timer_handler(); /* procesa LVGL */
        lv_port_indev_clear_buttons(); /* clear button states after processing */

        //= read_joystick_pitch();   /*   −100 … +100  */
        game_view_update();

        //	  uartSendString("joy X=");
        //	  uartSendValue(joy_raw[0]);
        //	  uartSendString(", joy Y=");
        //	  uartSendValue(joy_raw[1]);
        //	  uartSendString("\r\n");
        audio_task();          // <<--- acá

        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
                // OJO: el USER button de la NUCLEO es activo-bajo
                Audio_Beep(440.0f, 80);   // 1 kHz, 150 ms
                HAL_Delay(100);             // pequeño delay para que no dispare varias veces seguidas
            }

        HAL_Delay(5); /* 5 ms está bien (200 FPS máx) */
    /* USER CODE END WHILE */

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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */
////
  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */
////
  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */
////
  /* USER CODE END DAC_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
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
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 84-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 100-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
//static void MX_USART2_UART_Init(void)
//{
//
//  /* USER CODE BEGIN USART2_Init 0 */
////////    //
//  /* USER CODE END USART2_Init 0 */
//
//  /* USER CODE BEGIN USART2_Init 1 */
////////    //
//  /* USER CODE END USART2_Init 1 */
//  huart2.Instance = USART2;
//  huart2.Init.BaudRate = 115200;
//  huart2.Init.WordLength = UART_WORDLENGTH_8B;
//  huart2.Init.StopBits = UART_STOPBITS_1;
//  huart2.Init.Parity = UART_PARITY_NONE;
//  huart2.Init.Mode = UART_MODE_TX_RX;
//  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
//  if (HAL_UART_Init(&huart2) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN USART2_Init 2 */
////////    //
//  /* USER CODE END USART2_Init 2 */
//
//}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

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

  /*Configure GPIO pins : BTN_B_Pin BTN_A_Pin BTN_C_Pin BTN_D_Pin */
  GPIO_InitStruct.Pin = BTN_B_Pin|BTN_A_Pin|BTN_C_Pin|BTN_D_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/*-----------------------------------------------------------------------
 * Callback común de los 4 botones (PC0…PC3)
 *---------------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    const char* name = "UNK";

	switch (GPIO_Pin) {
	case BTN_A_Pin:   
		name = "A";  
		lv_port_indev_btn_a_pressed();
		break;
	case BTN_B_Pin:   
		name = "B";  
		lv_port_indev_btn_b_pressed();
		break;
	case BTN_C_Pin:   
		name = "C"; 
		lv_port_indev_btn_c_pressed();
		break;
	case BTN_D_Pin:   
		name = "D";    
		lv_port_indev_btn_d_pressed();
		break;
	default: return;                     /* otro pin, ignorar */
	}

    uartSendString("BTN ");
    uartSendString(name);
    uartSendString("\r\n");
}
/* ---------- ISR TIM6: HAL callback ---------- */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//  if (htim->Instance == TIM6) {
//    /* CASO A: beep cuadrado (BEEP_HZ → ponemos update_hz=2*BEEP_HZ) */
//#ifdef AUD_CASO_A
//    sq_toggle ^= 1;
//    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sq_toggle ? DAC_MAX : DAC_MIN);
//#endif
//
//    /* CASO B: wavetable por ISR (FS_HZ) */
//#ifdef AUD_CASO_B
//    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, wavetable[wt_idx++]);
//    if (wt_idx >= WT_LEN) wt_idx = 0;
//#endif
//  }
//}
/* ---------- Utilidad: genera seno 0..4095 ---------- */
//static void Wavetable_Init(void) {
//  for (uint32_t i=0;i<WT_LEN;i++) {
//    float s = sinf(2.0f * (float)M_PI * ((float)i/(float)WT_LEN));
//    uint16_t v = (uint16_t)(2048.0f + 1800.0f * s); // margen para no recortar
//    wavetable[i] = v;
//  }
//}



///* ---------- DAC sin trigger (para ISR que escribe el dato) ---------- */
//static void MX_DAC_Init_NoTrigger(void) {
//  __HAL_RCC_DAC_CLK_ENABLE();
//  hdac.Instance = DAC;
//  if (HAL_DAC_Init(&hdac) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  DAC_ChannelConfTypeDef sConfig = {0};
//  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;          // SIN TRIGGER
//  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
//  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//}
//
///* ---------- DAC con trigger TIM6 (para DMA) ---------- */
//static void MX_DAC_Init_TrigTIM6(void) {
//  __HAL_RCC_DAC_CLK_ENABLE();
//  hdac.Instance = DAC;
//  if (HAL_DAC_Init(&hdac) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  DAC_ChannelConfTypeDef sConfig = {0};
//  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;       // CON TRIGGER
//  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
//  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//}

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
    while (1) { }
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
