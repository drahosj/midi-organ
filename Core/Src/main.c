/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "scanchain.h"
#include "midi.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_EVENTS 16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static const struct scanchain_pindef chain_table[] = {
		{
				.chain_length = 9,
				.clrn_port = CHAIN1_CLRN_GPIO_Port,
				.clrn_pin = CHAIN1_CLRN_Pin,
				.lat_port = CHAIN1_LAT_GPIO_Port,
				.lat_pin = CHAIN1_LAT_Pin,
				.pln_port = CHAIN1_PLN_GPIO_Port,
				.pln_pin = CHAIN1_PLN_Pin,
				.hspi = &hspi1
		},
		{
				0,0,0,0,0,0,0,0
		}
};

uint8_t chain1_old[16];

/* Ghetto semaphore */
static volatile int flag;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int midi_scanchain(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
	unsigned char c = ch;
	HAL_UART_Transmit(&huart2, &c, 1, 10);

	return 0;
}

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
	if (pin == B1_Pin) {
		/* Set ghetto semaphore */
		flag = 1;
	}
}

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
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("Hello, world!\n");
  printf("Build: " __DATE__ " " __TIME__ "\n");

  printf("Initializing chains\n");
  int nchains = scanchain_init_all(chain_table);
  if (nchains > 0) {
	  printf("Initialized %d chain(s).\n", nchains);
  } else {
	  printf("Error with chain definitions or initialization!\n");
	  return -1;
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	 // printf("Waiting on interrupt event...\n");

	  while (flag == 0) {
		  HAL_Delay(50);
	  }

	  printf("Scanning...");
	  uint8_t scanbuf[16];
	  HAL_StatusTypeDef ret = scanchain_scan(&chain_table[0], scanbuf);
	  if (ret == HAL_OK) {
		  printf("Success!\n");
	  } else {
		  printf("Failed! (%d)\n", ret);
	  }
	  printf("CHAIN 1: ");
	  for (int i = 0; i < chain_table[0].chain_length; i++) {
		  /* Also invert the values for MIDI logic */
		  scanbuf[i] = scanbuf[i] ^ 0xff;

		  printf("%02X ", scanbuf[i]);
	  };
	  printf("\n");


	  struct midi_event events[MAX_EVENTS];
	  int nevents = midi_getevents(chain1_old, scanbuf, chain_table[0].chain_length, events, MAX_EVENTS);
	  memcpy(chain1_old, scanbuf, chain_table[0].chain_length);

	  if (nevents) {
		  uint8_t midibuf[MAX_EVENTS * 3];
		  printf("EVENTS: \n");
		  for (int i = 0; i < nevents; i++) {
			  /* Remap note numbers */
			  events[i].number = notemap1[events[i].number];
			  if (events[i].type == MIDI_NOTE_ON) {
				  printf("\t - NOTE_ON(%d)\n", events[i].number);
				  midibuf[i * 3] = 0x91;
				  midibuf[(i * 3) + 1] = events[i].number;
				  midibuf[(i * 3) + 2] = 0x7f;
			  } else if (events[i].type == MIDI_NOTE_OFF) {
				  printf("\t - NOTE_OFF(%d)\n", events[i].number);
				  midibuf[i * 3] = 0x81;
				  midibuf[(i * 3) + 1] = events[i].number;
				  midibuf[(i * 3) + 2] = 0x7f;
			  } else {
				  printf("\t - UNKNOWN(%d)\n", events[i].number);
			  }
		  }
		  printf("MIDI Buffer to Transmit: ");
		  for (int i = 0; i < nevents * 3; i++) {
			  printf("%02X ", midibuf[i]);
		  }
		  printf("\n");
		  ret = HAL_UART_Transmit(&huart1, midibuf, nevents * 3, 100);
		  if (ret) {
			  printf("UART Transmit Error %d\n", ret);
		  }
	  }

	  /* Free run */
	  //flag = 0;
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
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
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
