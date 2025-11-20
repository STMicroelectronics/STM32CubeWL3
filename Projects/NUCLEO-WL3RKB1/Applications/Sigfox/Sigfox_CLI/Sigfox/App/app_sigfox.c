/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_sigfox.c
  * @author  GPM WBL Application Team
  * @brief   Application of the Sigfox Middleware
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
#include "stm32_lpm.h"
#include "app_sigfox.h"

/* USER CODE BEGIN Includes */
#include "oob_adc.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIN(a,b)                        (((a) < (b))? (a) : (b))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
static void Fatal_Error(void);
static void SystemApp_Init(void);
static void Sigfox_Init(void);
PowerSaveLevels App_PowerSaveLevel_Check(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void MX_Sigfox_Init(void)
{
  /* USER CODE BEGIN MX_Sigfox_Init_1 */

  /* USER CODE END MX_Sigfox_Init_1 */
  SystemApp_Init();
  /* USER CODE BEGIN MX_Sigfox_Init_1_1 */

  /* USER CODE END MX_Sigfox_Init_1_1 */
  Sigfox_Init();
  /* USER CODE BEGIN MX_Sigfox_Init_2 */

  /* USER CODE END MX_Sigfox_Init_2 */
}

void MX_Sigfox_Process(void)
{
  /* USER CODE BEGIN MX_Sigfox_Process_1 */

  /* USER CODE END MX_Sigfox_Process_1 */

  /* USER CODE BEGIN MX_Sigfox_Process_2 */

  /* USER CODE END MX_Sigfox_Process_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/

/**
 * @brief  Application fatal error.
 * @param  None
 * @retval None
 */
static void Fatal_Error(void)
{
  /* USER CODE BEGIN Fatal_Error_1 */

  /* USER CODE END Fatal_Error_1 */

  while (1) {
  /* USER CODE BEGIN Fatal_Error_2 */

  /* USER CODE END Fatal_Error_2 */
  }
}

static void SystemApp_Init(void)
{
  /* USER CODE BEGIN SystemApp_Init_1 */

  /* Low Power Manager Init */
  UTIL_LPM_Init();  

  COM_InitTypeDef COM_Init = {0};
  
  COM_Init.BaudRate= 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.WordLength = COM_WORDLENGTH_8B;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  BSP_COM_Init(COM1, &COM_Init);
  
  LL_USART_EnableIT_RXNE(USART1);  
  
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif
  
  /* USER CODE END SystemApp_Init_1 */

  if (__HAL_RCC_MRSUBG_IS_CLK_DISABLED())
  {
    /* Radio Peripheral reset */
    __HAL_RCC_MRSUBG_FORCE_RESET();
    __HAL_RCC_MRSUBG_RELEASE_RESET();

    /* Enable Radio peripheral clock */
    __HAL_RCC_MRSUBG_CLK_ENABLE();
  }

  /* USER CODE BEGIN SystemApp_Init_2 */
  OOB_ADC_Init();  
  /* USER CODE END SystemApp_Init_2 */
}

static void Sigfox_Init(void)
{
  /* USER CODE BEGIN Sigfox_Init_1 */

  /* USER CODE END Sigfox_Init_1 */
  const ST_SFX_FEMConfiguration fem_configuration = { .model = ST_SFX_FEM_NONE };

  /* STM32WL3-Specific Sigfox Low-Level Initialization */
  ST_SFX_BoardCredentials sfxCredentials;
  ST_SFX_StatusTypeDef stSfxRetErr = ST_SFX_Init(&sfxCredentials, &fem_configuration);

  if (stSfxRetErr != ST_SFX_OK) {
    /* If an error occurred reading Sigfox credentials (for example the board has never been registered)
     * automatically set the test mode. */
    if (stSfxRetErr == ST_SFX_ERR_CREDENTIALS) {
      /* USER CODE BEGIN Sigfox_Init_2 */
      sfxCredentials.id = 0;
      memset(sfxCredentials.pac, 0x00, 8);
      sfxCredentials.rc = 0;

      set_testMode(1);
      /* USER CODE END Sigfox_Init_2 */
    } else
      Fatal_Error();
  }

  /* USER CODE BEGIN Sigfox_Init_3 */
  /* Set Sigfox configuration for the commands layer */
  set_id(sfxCredentials.id);
  set_pac(sfxCredentials.pac);
  set_rc(sfxCredentials.rc);

  /* User Application */
  if (get_testMode())
    printf("Sigfox CLI demo - TEST MODE (board not registered)\r\n");
  else
    printf("Sigfox CLI demo\r\n");

  for (uint16_t i = 0; i < sizeof(sfxCredentials.pac); i++) {
    printf("%.2X", sfxCredentials.pac[i]);
  }
  printf("\r\n");
  
  /* USER CODE END Sigfox_Init_3 */
}
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */
