/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_sdram.h"
#include "ov5640.h"
#include "rk043fn48h.h"
#include "fonts.h"

#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define QVGA_RES_X	320
#define QVGA_RES_Y	240

#define WQVGA_RES_X  480
#define WQVGA_RES_Y  272

#define VGA_RES_X  640
#define VGA_RES_Y  480

#define WVGA_RES_X  800
#define WVGA_RES_Y  480

#define ARGB8888_BYTE_PER_PIXEL  4
#define RGB565_BYTE_PER_PIXEL    2

/* --------------------------------RGB565 -------------------------------------*/
#define LCD_RGB565_COLOR_BLUE          0x001F
#define LCD_RGB565_COLOR_GREEN         0x07E0
#define LCD_RGB565_COLOR_RED           0xF800
#define LCD_RGB565_COLOR_CYAN          0x07FF
#define LCD_RGB565_COLOR_MAGENTA       0xF81F
#define LCD_RGB565_COLOR_YELLOW        0xFFE0
#define LCD_RGB565_COLOR_LIGHTBLUE     0x841F
#define LCD_RGB565_COLOR_LIGHTGREEN    0x87F0
#define LCD_RGB565_COLOR_LIGHTRED      0xFC10
#define LCD_RGB565_COLOR_LIGHTCYAN     0x87FF
#define LCD_RGB565_COLOR_LIGHTMAGENTA  0xFC1F
#define LCD_RGB565_COLOR_LIGHTYELLOW   0xFFF0
#define LCD_RGB565_COLOR_DARKBLUE      0x0010
#define LCD_RGB565_COLOR_DARKGREEN     0x0400
#define LCD_RGB565_COLOR_DARKRED       0x8000
#define LCD_RGB565_COLOR_DARKCYAN      0x0410
#define LCD_RGB565_COLOR_DARKMAGENTA   0x8010
#define LCD_RGB565_COLOR_DARKYELLOW    0x8400
#define LCD_RGB565_COLOR_WHITE         0xFFFF
#define LCD_RGB565_COLOR_LIGHTGRAY     0xD69A
#define LCD_RGB565_COLOR_GRAY          0x8410
#define LCD_RGB565_COLOR_DARKGRAY      0x4208
#define LCD_RGB565_COLOR_BLACK         0x0000
#define LCD_RGB565_COLOR_BROWN         0xA145
#define LCD_RGB565_COLOR_ORANGE        0xFD20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x2004c000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x2004c0a0
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x2004c000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x2004c0a0))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */

#endif

ETH_TxPacketConfig TxConfig;

ADC_HandleTypeDef hadc3;

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

DMA2D_HandleTypeDef hdma2d;

ETH_HandleTypeDef heth;

I2C_HandleTypeDef hi2c1;

LTDC_HandleTypeDef hltdc;

QSPI_HandleTypeDef hqspi;

RTC_HandleTypeDef hrtc;

SAI_HandleTypeDef hsai_BlockA2;
SAI_HandleTypeDef hsai_BlockB2;

SD_HandleTypeDef hsd1;

SPDIFRX_HandleTypeDef hspdif;

SPI_HandleTypeDef hspi2;

SDRAM_HandleTypeDef hsdram1;

/* USER CODE BEGIN PV */
/* Private variables	*/
static volatile uint32_t start_the_camera_capture = 0;
typedef enum
{
	CAMERA_OK = 0x00, CAMERA_ERROR = 0x01, CAMERA_TIMEOUT = 0x02, CAMERA_NOT_DETECTED = 0x03, CAMERA_NOT_SUPPORTED = 0x04
} Camera_StatusTypeDef;

typedef struct
{
	uint32_t TextColor; uint32_t BackColor; sFONT *pFont;
}LCD_DrawPropTypeDef;

typedef struct
{
	int16_t X; int16_t Y;
}Point, * pPoint;

static LCD_DrawPropTypeDef DrawProp[2];
LTDC_HandleTypeDef hltdc;
LTDC_LayerCfgTypeDef layer_cfg;
static RCC_PeriphCLKInitTypeDef periph_clk_init_struct;
CAMERA_DrvTypeDef *camera_driv;
/* Camera module I2C HW address */
static uint32_t CameraHwAddress;
/* Image size */
uint32_t Im_size = 0;

/* Camera resolution is QWVGA (480x272) */
static uint32_t   CameraResX = WQVGA_RES_X;
static uint32_t   CameraResY = WQVGA_RES_Y;
static uint32_t   LcdResX    = WQVGA_RES_X; /* QWVGA landscape */
static uint32_t   LcdResY    = WQVGA_RES_Y;

static uint32_t          offset_cam = 0;
static uint32_t          offset_lcd = 0;

__IO uint16_t pBuffer[WQVGA_RES_X * WQVGA_RES_Y];
HAL_StatusTypeDef hal_status = HAL_OK;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

uint8_t CAMERA_Init(uint32_t );
void CAMERA_PwrDown(void);
void CAMERA_PwrUp(void);

static void LTDC_Init(uint32_t , uint16_t , uint16_t , uint16_t, uint16_t);
void LCD_GPIO_Init(LTDC_HandleTypeDef *, void *);
static void OnError_Handler(uint32_t condition);

/* USER CODE END PFP */

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

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

	CAMERA_PwrDown();
	hal_status = BSP_SDRAM_Init();
	OnError_Handler(hal_status != HAL_OK);

	/*##-3- Camera Initialization and start capture ############################*/
	/* Initialize the Camera in WQVGA mode */
	  /* Init camera default resolution and format */
	//hal_status = BSP_CAMERA_Init(CAMERA_R320x240);
	CAMERA_PwrUp();
	HAL_Delay(1000);
	//hal_status = CAMERA_Init(CAMERA_R320x240);
	hal_status = CAMERA_Init(CAMERA_R480x272);

	OnError_Handler(hal_status != HAL_OK);

	// HAL_Delay(1000); //Delay for the camera to output correct data
	Im_size = 0xff00;  //size=480*272*2/4

	/*##-4- Camera Continuous capture start in QVGA resolution ############################*/
	/* Disable unwanted HSYNC (IT_LINE)/VSYNC interrupts */
	__HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC);
	//__HAL_DCMI_ENABLE_IT(&hdcmi, DCMI_IT_FRAME);

	/* uncomment the following line in case of snapshot mode */
	//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)CAMERA_FRAME_BUFFER, Im_size);
	/* uncomment the following line in case of continuous mode */
	hal_status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS , (uint32_t)CAMERA_FRAME_BUFFER, Im_size);
	OnError_Handler(hal_status != HAL_OK);

	LTDC_Init(FRAME_BUFFER, 0, 0, 480, 272);

//	hal_status = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT , (uint32_t)pBuffer, CameraResX * CameraResY/2);
//	OnError_Handler(hal_status != HAL_OK);

//	/*##-4- Frame display ############################*/
//	/* Wait until LCD frame buffer is ready */
//	while(start_the_camera_capture == 0) {;}
//
//	__IO uint16_t uval=0xffff;
//	for(__IO uint32_t  uwIndex = 0; uwIndex < 64; uwIndex++)
//	{
//		uval=pBuffer[uwIndex];
//		printf("CAM pBuffer[%lu] = %u\n", uwIndex, uval);
//	}
//
//	printf("\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SAI2
                              |RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DCMI Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCMI_Init(void)
{

  /* USER CODE BEGIN DCMI_Init 0 */

  /* USER CODE END DCMI_Init 0 */

  /* USER CODE BEGIN DCMI_Init 1 */

  /* USER CODE END DCMI_Init 1 */
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&hdcmi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DCMI_Init 2 */

  /* USER CODE END DCMI_Init 2 */

}


/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00C0EAFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, ARDUINO_D7_Pin|ARDUINO_D8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DISP_GPIO_Port, LCD_DISP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DCMI_PWR_EN_GPIO_Port, DCMI_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, ARDUINO_D4_Pin|ARDUINO_D2_Pin|EXT_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : OTG_HS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_HS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_HS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_D7_Pin ULPI_D6_Pin ULPI_D5_Pin ULPI_D3_Pin
                           ULPI_D2_Pin ULPI_D1_Pin ULPI_D4_Pin */
  GPIO_InitStruct.Pin = ULPI_D7_Pin|ULPI_D6_Pin|ULPI_D5_Pin|ULPI_D3_Pin
                          |ULPI_D2_Pin|ULPI_D1_Pin|ULPI_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_VBUS_Pin */
  GPIO_InitStruct.Pin = OTG_FS_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Audio_INT_Pin */
  GPIO_InitStruct.Pin = Audio_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Audio_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_D7_Pin ARDUINO_D8_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = ARDUINO_D7_Pin|ARDUINO_D8_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TP3_Pin NC2_Pin */
  GPIO_InitStruct.Pin = TP3_Pin|NC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : DCMI_PWR_EN_Pin */
  GPIO_InitStruct.Pin = DCMI_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DCMI_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_INT_Pin */
  GPIO_InitStruct.Pin = LCD_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_NXT_Pin */
  GPIO_InitStruct.Pin = ULPI_NXT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_NXT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_D4_Pin ARDUINO_D2_Pin EXT_RST_Pin */
  GPIO_InitStruct.Pin = ARDUINO_D4_Pin|ARDUINO_D2_Pin|EXT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_STP_Pin ULPI_DIR_Pin */
  GPIO_InitStruct.Pin = ULPI_STP_Pin|ULPI_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_RXER_Pin */
  GPIO_InitStruct.Pin = RMII_RXER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RMII_RXER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_CLK_Pin ULPI_D0_Pin */
  GPIO_InitStruct.Pin = ULPI_CLK_Pin|ULPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void LCD_GPIO_Init(LTDC_HandleTypeDef *hltdc, void *Params)
{
	GPIO_InitTypeDef gpio_init_structure;
	/* Enable the LTDC and DMA2D clocks */
	__HAL_RCC_LTDC_CLK_ENABLE();
	__HAL_RCC_DMA2D_CLK_ENABLE();
	/* Enable GPIOs clock */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();
	/*** LTDC Pins configuration ***/
	/* GPIOE configuration */
	gpio_init_structure.Pin = GPIO_PIN_4;
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull = GPIO_NOPULL;
	gpio_init_structure.Speed = GPIO_SPEED_FAST;
	gpio_init_structure.Alternate = GPIO_AF14_LTDC; HAL_GPIO_Init(GPIOE, &gpio_init_structure);
	/* GPIOG configuration */
	gpio_init_structure.Pin = GPIO_PIN_12;
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Alternate = GPIO_AF9_LTDC;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);
	/* GPIOI LTDC alternate configuration */
	gpio_init_structure.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio_init_structure.Mode = GPIO_MODE_AF_PP; gpio_init_structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOI, &gpio_init_structure);
	/* GPIOJ configuration */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
			GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_5
			| GPIO_PIN_6 | GPIO_PIN_7 |GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOJ, &gpio_init_structure);
	/* GPIOK configuration */
	gpio_init_structure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	gpio_init_structure.Mode = GPIO_MODE_AF_PP;
	gpio_init_structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOK, &gpio_init_structure);
	/* LCD_DISP GPIO configuration */
	gpio_init_structure.Pin = GPIO_PIN_12; /* LCD_DISP pin has to be manually controlled */
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOI, &gpio_init_structure);
	/* LCD_BL_CTRL GPIO configuration */
	gpio_init_structure.Pin = GPIO_PIN_3; /* LCD_BL_CTRL pin has to be manually controlled */
	gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOK, &gpio_init_structure);
}

static void LTDC_Init(uint32_t FB_Address, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
	/* Timing Configuration */
	hltdc.Init.HorizontalSync = (RK043FN48H_HSYNC - 1);
	hltdc.Init.VerticalSync = (RK043FN48H_VSYNC - 1);
	hltdc.Init.AccumulatedHBP = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	hltdc.Init.AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hltdc.Init.AccumulatedActiveH = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hltdc.Init.AccumulatedActiveW = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	hltdc.Init.TotalHeigh = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
	hltdc.Init.TotalWidth = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);

	/* LCD clock configuration */
	periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	periph_clk_init_struct.PLLSAI.PLLSAIN = 192;
	periph_clk_init_struct.PLLSAI.PLLSAIR = RK043FN48H_FREQUENCY_DIVIDER;
	periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
	HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);

	/* Initialize the LCD pixel width and pixel height */
	hltdc.LayerCfg->ImageWidth = RK043FN48H_WIDTH;
	hltdc.LayerCfg->ImageHeight = RK043FN48H_HEIGHT;
	hltdc.Init.Backcolor.Blue = 0;

	/* Background value */
	hltdc.Init.Backcolor.Green = 0;
	hltdc.Init.Backcolor.Red = 0;

	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc.Instance = LTDC;
	if(HAL_LTDC_GetState(&hltdc) == HAL_LTDC_STATE_RESET)
	{
		LCD_GPIO_Init(&hltdc, NULL);
	}
	HAL_LTDC_Init(&hltdc);

	/* Assert display enable LCD_DISP pin */
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);
	/* Assert backlight LCD_BL_CTRL pin */
	HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);
	DrawProp[0].pFont = &Font24 ;
	/* Layer Init */
	layer_cfg.WindowX0 = Xpos;
	layer_cfg.WindowX1 = Width;
	layer_cfg.WindowY0 = Ypos;
	layer_cfg.WindowY1 = Height;
	layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	layer_cfg.FBStartAdress = FB_Address;
	layer_cfg.Alpha = 255;
	layer_cfg.Alpha0 = 0;
	layer_cfg.Backcolor.Blue = 0;
	layer_cfg.Backcolor.Green = 0;
	layer_cfg.Backcolor.Red = 0;
	layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	layer_cfg.ImageWidth = Width;
	layer_cfg.ImageHeight = Height;
	HAL_LTDC_ConfigLayer(&hltdc, &layer_cfg, 1);

	DrawProp[1].BackColor = ((uint32_t)0xFFFFFFFF);
	DrawProp[1].pFont = &Font24;
	DrawProp[1].TextColor = ((uint32_t)0xFF000000);
}

uint8_t CAMERA_Init(uint32_t Resolution) /*Camera initialization*/
{
	uint8_t status = CAMERA_ERROR;
	/* Read ID of Camera module via I2C */
	if(ov5640_ReadID(CAMERA_I2C_ADDRESS) == OV5640_ID)
	{
		camera_driv = &ov5640_drv;
		/* Initialize the camera driver structure */
		CameraHwAddress = CAMERA_I2C_ADDRESS;
		if (Resolution == CAMERA_R320x240)
		{
			camera_driv->Init(CameraHwAddress, Resolution);
			HAL_DCMI_DisableCROP(&hdcmi);
		    HAL_Delay(500);

		}
		if (Resolution == CAMERA_R480x272)
		{
			camera_driv->Init(CameraHwAddress, Resolution);
			HAL_DCMI_DisableCROP(&hdcmi);
		    HAL_Delay(500);
	    }

		status = CAMERA_OK; /* Return CAMERA_OK status */
	}
	else
	{
		status = CAMERA_NOT_SUPPORTED; /* Return CAMERA_NOT_SUPPORTED status */
	}
	return status;
}

/**
  * @brief  CANERA power up
  * @retval None
  */
void CAMERA_PwrUp(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable GPIO clock */
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_13;
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

  /* De-assert the camera POWER_DOWN pin (active high) */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_13, GPIO_PIN_RESET);

  HAL_Delay(3);     /* POWER_DOWN de-asserted during 3ms */
}

/**
  * @brief  CAMERA power down
  * @retval None
  */
void CAMERA_PwrDown(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable GPIO clock */
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*** Configure the GPIO ***/
  /* Configure DCMI GPIO as alternate function */
  gpio_init_structure.Pin       = GPIO_PIN_13;
  gpio_init_structure.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull      = GPIO_NOPULL;
  gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

  /* Assert the camera POWER_DOWN pin (active high) */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
 * @brief  Converts a line to an ARGB8888 pixel format.
 * @param  pSrc: Pointer to source buffer
 * @param  pDst: Output color
 * @param  ColorMode: Input color mode
 * @retval None
 */
static void LCD_LL_ConvertLineToARGB8888(void *pSrc, void *pDst)
{
	/* Enable DMA2D clock */
	__HAL_RCC_DMA2D_CLK_ENABLE();

	/* Configure the DMA2D Mode, Color Mode and output offset */
	hdma2d.Init.Mode         = DMA2D_M2M_PFC;
	hdma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB565; // DMA2D_OUTPUT_ARGB8888; /* Output color out of PFC */
	hdma2d.Init.OutputOffset = 0;
	//hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
	//hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

	// DMA2D_INPUT_ARGB8888        0x00000000U  /*!< ARGB8888 color mode */
	// DMA2D_INPUT_RGB888          0x00000001U  /*!< RGB888 color mode   */
	// DMA2D_INPUT_RGB565          0x00000002U  /*!< RGB565 color mode   */
	// DMA2D_INPUT_ARGB1555        0x00000003U  /*!< ARGB1555 color mode */
	// DMA2D_INPUT_ARGB4444        0x00000004U  /*!< ARGB4444 color mode */
	// DMA2D_INPUT_L8              0x00000005U  /*!< L8 color mode       */
	// DMA2D_INPUT_AL44            0x00000006U  /*!< AL44 color mode     */
	// DMA2D_INPUT_AL88            0x00000007U  /*!< AL88 color mode     */
	// DMA2D_INPUT_L4              0x00000008U  /*!< L4 color mode       */
	// DMA2D_INPUT_A8              0x00000009U  /*!< A8 color mode       */
	// DMA2D_INPUT_A4              0x0000000AU  /*!< A4 color mode       */

	/* Foreground Configuration */
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF; /* fully opaque */
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
	hdma2d.LayerCfg[1].InputOffset = 0;
	//hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
	//hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */

	hdma2d.Instance = DMA2D;

	/* DMA2D Initialization */
	if(HAL_DMA2D_Init(&hdma2d) == HAL_OK)
	{
		if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
		{
			if (HAL_DMA2D_Start(&hdma2d, (uint32_t)pSrc, (uint32_t)pDst, CameraResX, 1) == HAL_OK)
			{
				/* Polling For DMA transfer */
				HAL_DMA2D_PollForTransfer(&hdma2d, 10);
			}
		}
	}
	else
	{
		while(1);
	}
}

/**
 * @brief  Camera Frame Event callback.
 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	start_the_camera_capture = 1;
}

/**
 * @brief  Camera line event callback
 * @param  None
 * @retval None
 */
//void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
//{
//	if (offset_lcd == 0) // ASSUMPTION Camera resolution is lower than LCD resolution
//	{
//		/* If Camera resolution is lower than LCD resolution, set display in the middle of the screen */
//		offset_lcd =   ((((LcdResY - CameraResY) / 2) * LcdResX)   /* Middle of the screen on Y axis */
//				+   ((LcdResX - CameraResX) / 2))             /* Middle of the screen on X axis */
//    				  * sizeof(uint32_t);
//	}
//
//	for(uint32_t camera_line_nb = 0; camera_line_nb < CameraResY; camera_line_nb++)
//	{
////		LCD_LL_ConvertLineToARGB8888((uint32_t *)(CAMERA_FRAME_BUFFER + offset_cam),
////				(uint32_t *)(LCD_FRAME_BUFFER + offset_lcd));
//
//		for (uint32_t ncol=0; ncol<CameraResX; ncol ++){
//			uint32_t val = (CAMERA_FRAME_BUFFER + offset_cam+4*ncol);
//			*(__IO uint32_t*)(4*ncol+LCD_FRAME_BUFFER) = val;
//		}
//
//		offset_cam  = offset_cam + (CameraResX * sizeof(uint16_t));
//		offset_lcd  = offset_lcd + (LcdResX * sizeof(uint32_t));
//	}
//}

static void test_ConvertLineRGB565ToARGB8888(){
	// white/magenta test color pattern
	uint32_t offset_cam=0;
	uint32_t offset_lcd=0;

	for (uint32_t pos=0; pos<LcdResX*LcdResY/2; pos ++){
		uint32_t val = LCD_RGB565_COLOR_WHITE << 16 | LCD_RGB565_COLOR_DARKMAGENTA;
		*(__IO uint32_t*)(4*pos+CAMERA_FRAME_BUFFER) = val;
	}

	for(uint32_t camera_line_nb = 0; camera_line_nb < CameraResY; camera_line_nb++){
		LCD_LL_ConvertLineToARGB8888((uint32_t *)(CAMERA_FRAME_BUFFER + offset_cam),
				(uint32_t *)(LCD_FRAME_BUFFER + offset_lcd));

		offset_cam  = offset_cam + (CameraResX * sizeof(uint16_t));
		offset_lcd  = offset_lcd + (LcdResX * sizeof(uint32_t));
	}
}

/**
 * @brief  On Error Handler on condition TRUE.
 * @param  condition : Can be TRUE or FALSE
 * @retval None
 */
static void OnError_Handler(uint32_t condition)
{
	if(condition)
	{
		BSP_LED_On(LED1);
		while(1) { ; } /* Blocking on error */
	}
}
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
