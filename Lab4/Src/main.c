/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "math.h"

#ifdef __stdio_h
#include <stdio.h>
#endif

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __segment_display_h
#include "segment_display.h"
#endif

#ifndef __fsm_h
#include "fsm.h"
#endif

#ifndef __adc_thread_h
#include "adc_thread.h"
#endif


#define ABS(x) ((x < 0)? -x : x)
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define BOUND(x, lower, upper) (MAX(MIN(x, upper), lower))

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

const int PWM_TIMER_PERIOD = 1680;




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

// Called in order to adjust the duty cycle.
void adjust_duty_cycle(float current_rms);

// starts the ADC.
void start_adc(void);
// stops the ADC.
void stop_adc(void);

extern uint32_t ADCBufferDMA[];

extern void adc_buffer_full_callback(void);

// Function that is called whenever the blue button is pressed.
void button_pressed_callback(void);
void pwm_duty_cycle(uint16_t new_period);


/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/** @brief Set the period of the PWM timer
* @param new_period: new timer period.
*/
void pwm_duty_cycle(uint16_t new_period) //input percentage
{
//    uint16_t value = (uint16_t)(PWM_TIMER_PERIOD)*percentage; //(period)*(percent/100)
		TIM_OC_InitTypeDef sConfigOC;
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = BOUND(new_period, 0, PWM_TIMER_PERIOD);
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  
}


/** @brief Controller which adjusts the PWM duty cycle in order to match the current target RMS voltage.
* @param current_rms: The current RMS voltage from the ADC.
*/
void adjust_duty_cycle(float current_rms){ 
	extern float target_voltage;
	// a damping constant, that limits the rate of change of the percentage.
	static const float damping = 0.005f;
	
//	static const float increment = PWM_TIMER_PERIOD / 3.0f;
	static float current_percentage;
	static int current_period;
	static float difference;
	
	difference = current_rms - target_voltage;
	
	current_percentage -= damping * difference;
	current_percentage = BOUND(current_percentage, 0.f, 1.f);
	
	
	current_period = round(current_percentage * PWM_TIMER_PERIOD);
	
	printf("Current voltage: %2.3f, Target Voltage: %2.3f, current percentage: %2.5f%%, current_period: %u / %u \n", current_rms, target_voltage, current_percentage*100, current_period, PWM_TIMER_PERIOD);
	pwm_duty_cycle(current_period);
}

void start_adc(){
	HAL_ADC_Start_DMA(&hadc1, ADCBufferDMA, ADC_BUFFER_SIZE);
}

void stop_adc(){
	HAL_ADC_Stop_DMA(&hadc1);
}

// Called when the buffer is filled.
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	
	if(AdcHandle->Instance == ADC1){
		adc_buffer_full_callback();
		stop_adc();
		start_adc();
	}
}
/** Called whenever an EXTI interrupt occurs (i.e. button press)
*
*/
void HAL_GPIO_EXTI_Callback(uint16_t pin){
	switch(pin){
		case BLUE_BUTTON_Pin:
			button_pressed_callback();
			break;
		default:
			printf("EXTI interrupt!\n");
	}
		
}

 void button_pressed_callback(){
	extern uint8_t display_mode;
	display_mode++;
	display_mode %= 3;
	switch(display_mode){
		case DISPLAY_RMS:
			// TODO: display one of the LEDs
			HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LD5_Pin, GPIO_PIN_RESET);
			break;
		case DISPLAY_MIN:
			HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, LD5_Pin, GPIO_PIN_RESET);
			break;
		case DISPLAY_MAX:
			HAL_GPIO_WritePin(GPIOD, LED3_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED4_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LD5_Pin, GPIO_PIN_SET);
			break;
	}
}





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_DAC_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	
	
	// Start the timers.
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3); 
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	
  HAL_TIM_Base_Start_DMA(&htim2, ADCBufferDMA, ADC_BUFFER_SIZE);
	
	sleep();
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
