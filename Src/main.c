/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void proccesDmaData(uint8_t sign);

//global variables
int start=0;
int koniec=0;
int poc_prijatych=0;
uint8_t pole[10];
int mode_auto=0;			// 0= manual 1=auto
int pwm_cnt=0;				// PWM

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  USART2_RegisterCallback(proccesDmaData);

  	  	  	uint8_t tx1[] = "Buffer capacity: ";
    		uint8_t tx2[] = " bytes, occupied memory: ";
    		uint8_t tx3[] = " bytes, load [in %]: ";
    		uint8_t tx4[] = "  %\n \r";

    		while (1)
  {
    					USART2_CheckDmaReception();

    					int occupied = numOfOccupied();
    					int capacity = sizeOfBuff();
    					float occupied1 = occupied;
    					float percentage = occupied1/capacity*100.0;
    					uint8_t final[100];
    					uint8_t pomoc1[10],pomoc2[10],pomoc3[10];
    					memset(final,0,sizeof(final));
    					itoa(capacity,pomoc1,10);
    					strcpy(final,tx1);
    					strcat(final, pomoc1);
    					itoa(occupied,pomoc2,10);
    					strcat(final,tx2);
    					strcat(final, pomoc2);
    					strcat(final,tx3);
    					gcvt(percentage, 4, pomoc3);
    					strcat(final,pomoc3);
    					strcat(final,tx4);

    				  USART2_PutBuffer(final, strlen(final));

    				  LL_mDelay(800);
  }

}

/**
  * @brief System Clock Configuration
  * @retval None
  */

void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();
  }
  LL_RCC_HSI_Enable();


  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_4);
  LL_RCC_PLL_Enable();


  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);


  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
  LL_RCC_SetADCClockSource(LL_RCC_ADC12_CLKSRC_PLL_DIV_1);
}

void proccesDmaData(uint8_t sign)
{
	/* Process received data */
		// type your algorithm here:

		if(start==1 && sign!=0 && sign !='$'){					//nacitavanie slova
			pole[poc_prijatych]=sign;
			poc_prijatych++;
		}

		if ((poc_prijatych==10 || (sign=='$' && poc_prijatych<10)) && start == 1)		//ked doslo moc zbytocnych znakov / ukoncovaci
		{

					poc_prijatych=0;

					if (sign == '$') 	//iba ked je ukoncovaci
					{
						checkForKeyWords();
					}
					memset(pole, 0, strlen(pole));
		}

		if (start==1 &&  sign=='$'){
					koniec=1;
					start=0;
				}

		if (start==0 && sign=='$' && koniec==0){				// zaciatok
					start=1;
				}


}

void checkForKeyWords(){					//
	const uint8_t autoWord[] = "auto";
	const uint8_t manWord[] = "manual";
	const uint8_t pwmWord[] = "pwm";

	int dlzka = strlen(pole);
	for (int i = 0; i < dlzka ; i++)
	{
		pole[i] = tolower(pole[i]);		//da to na male
	}
	char *pomocny_pointer;

	//checkneme hotwordy
	if (!strcmp(pole, autoWord))
		{
		mode_auto = 1;
		koniec=0;
		}
	if (!strcmp(pole, manWord))
		{
		mode_auto = 0;
		koniec=0;
		}

	pomocny_pointer = strstr(pole, pwmWord);
	char uroven[2];
	if (pomocny_pointer != NULL)
		{
		if (pole[3] != 0)
		{
			uroven[0] =	pole[3];
			uroven[1] = pole[4];
		}
		else
			uroven[0] = pole[4];
		pwm_cnt = atoi(uroven);
		koniec=0;
		}

}






/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT
void assert_failed(char *file, uint32_t line)
{
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
