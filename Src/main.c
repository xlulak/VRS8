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
int poc_prijatych=0;
uint8_t pole[10];
int mode_auto=1;			// 0= manual 1=auto
int pwm_cnt=0;				// PWM uroven
int CH1_DC = 0;
int pom=0;
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
  //MX_ADC1_Init();
  MX_TIM2_Init();
  USART2_RegisterCallback(proccesDmaData);

  	  		uint8_t tx1[] = "Current mode: ";
    		uint8_t tx2[] = " PWM: ";
    		uint8_t tx3[] = "[in %]\n \r";
    		uint8_t autovypis[] = "auto";
    		uint8_t manvypis[] = "manual";
    		uint8_t pwmvypis[10];
    		uint8_t final[50];
    		while (1)
  {
    					USART2_CheckDmaReception();
    					memset(final,0,sizeof(final));
    					strcpy(final,tx1);
    					if (mode_auto == 1)
    					{
    						strcat(final,autovypis);
    					}
    					else
    					{
    						strcat(final,manvypis);
    					}
    					strcat(final,tx2);
    					itoa(pwm_cnt,pwmvypis,10);
    					strcat(final,pwmvypis);
    					strcat(final,tx3);
    				    USART2_PutBuffer(final, strlen(final));

    			if (mode_auto == 1){

    				while(CH1_DC < 99)
    			    	{
    			    	    CH1_DC += 1;
    			    	    LL_mDelay(10);
    			    	}
    			    	while(CH1_DC >= 0)
    			    	{
    			    	    CH1_DC -= 1;
    			    	    LL_mDelay(10);
    			    	}

    			} else{
    										while(CH1_DC < 99)
    				    			    	{
    				    			    	    CH1_DC += 1;
    				    			    	    LL_mDelay(10);
    				    			    	}
    				    			    	while(CH1_DC >= pwm_cnt)
    				    			    	{
    				    			    	    CH1_DC -= 1;
    				    			    	    LL_mDelay(10);
    				    			    	}
    			}
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
	// funkcia prijma len 1 znak, preto treba najprv prejst spracovanim

		if(start == 1 && sign != 0 && sign != '$'){					//nacitavanie slova -> ignoruje prazdne znaky a startovaci/konecny
			pole[poc_prijatych] = sign;								//pridaj do nacitavaneho pola novy znak
			poc_prijatych++;										//pridaj pocet prijatych
		}

		if (sign=='$' && start == 1)
		{

			poc_prijatych=0;					//vynuluje pocet prijatych
			checkForKeyWords();					//skontroluje, ci nebolo prijate nieco na zmenu nastaveni
			memset(pole, 0, strlen(pole));		//vymaze prijate pole
			start = 0;							//vypne signalizaciu prijatia prveho znaku
			return;								//vypne vykonavanie funkcie, kedze preslo kontrolou
		}

		if (poc_prijatych == 10 && start == 1)	//timeout -> prisli zbytocne znaky
		{
			poc_prijatych=0;					//vynuluje pocet prijatych
			start = 0;							//vypne signalizaciu prijatia prveho znaku
			memset(pole, 0, strlen(pole));		//vymaze prijate pole
			return;								//vypne vykonavanie funkcie, kedze dosiel timeout
		}

		if (start==0 && sign=='$')				// prisiel prvy znak a este nie je zapnuty start
		{
			start=1;							//zapni start
		}


}

void checkForKeyWords()
{
	//kontroluje keywordy auto, manual a pwmXX

	//konstantne polia znakov, ktore su keywordami
	const uint8_t autoWord[] = "auto";
	const uint8_t manWord[] = "manual";
	const uint8_t pwmWord[] = "pwm";

	//premena pola na male znaky, aby to nebolo case sensitive
	int dlzka = strlen(pole);
	for (int i = 0; i < dlzka ; i++)
	{
		pole[i] = tolower(pole[i]);		//tolower zmeni pismenka na male, ostatne cleny (cisla, znamienka ignoruje)
	}



	//checkneme hotwordy
	if (!strcmp(pole, autoWord))		//check ak pride auto
		{
		mode_auto = 1;					//tak zapni auto rezim
		pwm_cnt=0;						// ked prepneme na auto bude 0
		}
	if (!strcmp(pole, manWord))			//ak pride manual
		{
		mode_auto = 0;					//tak vypni auto rezim
		}

	//pomocne na vykonanie PWM checku
	char *pomocny_pointer;
	char uroven[2];

	pomocny_pointer = strstr(pole, pwmWord);	//vrati pointer na lokaciu stringu pwmWord v poli, ak tam nie je da NULL

	if ((pomocny_pointer != NULL) && (mode_auto == 0))				//neni null, tak znamena ze command PWM bol najdeny + ak sme v manualnom mode
		{
		if (pole[3] != '0')						//ak neni stvrty znak 0 tak nacita obe cisla		pwm -> pole[0],pole[1],pole[2]; pole[3] je teda prve cislo
		{
			uroven[0] =	pole[3];
			uroven[1] = pole[4];
		}
		else
			uroven[0] = pole[4];				//ak nahodou to bola nula, precita len druhe -> napr z 08 tak precita len 8
		pwm_cnt = atoi(uroven);					//premeni na int a ulozi to globalnej premennej pwm_cnt
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
