/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"

/* USER CODE BEGIN 0 */
#ifndef DISPLAY_RMS 
#include "heads_up_display.h"
#endif

#ifndef KEYPAD
#include "keypad.h"
#endif

#ifndef bool
#include <stdbool.h>
#endif


// Function used to refresh the display.
void refresh_display(void);


// Function called periodically to check if a digit was pressed.
void check_for_digit_press(void);

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
  /* USER CODE BEGIN HardFault_IRQn 1 */

  /* USER CODE END HardFault_IRQn 1 */
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
  /* USER CODE BEGIN MemoryManagement_IRQn 1 */

  /* USER CODE END MemoryManagement_IRQn 1 */
}

/**
* @brief This function handles Pre-fetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
  /* USER CODE BEGIN BusFault_IRQn 1 */

  /* USER CODE END BusFault_IRQn 1 */
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
  /* USER CODE BEGIN UsageFault_IRQn 1 */

  /* USER CODE END UsageFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
	// NOTE: This function gets called every 1ms.
	static const int ms_per_systick = 1;
	
	// target sampling frequency of the ADC (in Hz)
//	static const int target_ADC_sampling_freq = 50;
//	static const int target_ADC_sampling_period = (1000 / target_ADC_sampling_freq);
//		
	// Threshold for the display counter. When reached, the display is refreshed.
	static const int systicks_per_display_refresh = DISPLAY_REFRESH_INTERVAL_MS / ms_per_systick;
	// Counter for refreshing the display.
	static int refresh_display_counter;

	
	static const int systicks_per_check_for_digit_press = CHECK_FOR_DIGIT_PRESS_INTERVAL_MS / ms_per_systick;
	static int check_for_digit_press_counter;
	
	static const int systicks_per_pwm_update = 1000/ms_per_systick;
	static int pwm_counter;
	static uint16_t percentage;
	
	// TODO: we might remove this when we get the timers to work properly.
//	// Threshold for the ADC counter. When reached, the ADC is sampled.
//	static const int systicks_per_ADC_sample = target_ADC_sampling_period / ms_per_systick;
//	// Counter for sampling the ADC.
//	static int sample_ADC_counter;
	

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

	//TODO: get pwm_duty_cycle to be called in this interrupt
	//update DutyCycle
	pwm_counter++;
	if(pwm_counter == systicks_per_pwm_update){
		percentage = (percentage+10)%100;
		//pwm_duty_cycle(percentage);
		pwm_counter = 0;
	}

	// Refresh the display when appropriate.
	refresh_display_counter++;
	if (refresh_display_counter == systicks_per_display_refresh){
		refresh_display();
		refresh_display_counter = 0;
	}
	
	// Check for a digit press when appropriate.
	check_for_digit_press_counter++;
	if (check_for_digit_press_counter == systicks_per_check_for_digit_press){
		check_for_digit_press();
		check_for_digit_press_counter = 0;
	}
	
//	// Sample the ADC when appropriate.
//	sample_ADC_counter++;
//	if (sample_ADC_counter == systicks_per_ADC_sample){
//		// Start the ADC Interrupt Routine.		
//		HAL_ADC_Start_IT(&hadc1);
//		sample_ADC_counter = 0;
//	}
	


  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles EXTI line0 interrupt.
*/
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */
  /* USER CODE END EXTI0_IRQn 1 */
}

/**
* @brief This function handles ADC1, ADC2 and ADC3 global interrupts.
*/
void ADC_IRQHandler(void)
{
  /* USER CODE BEGIN ADC_IRQn 0 */

  /* USER CODE END ADC_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC_IRQn 1 */

  /* USER CODE END ADC_IRQn 1 */
}

/**
* @brief This function handles TIM2 global interrupt.
*/
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
	printf("TIMER IRQ HANDLER\n");
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
* @brief This function handles TIM3 global interrupt.
*/
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
	printf("Timer3 interrupt");
  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
* @brief This function handles DMA2 stream0 global interrupt.
*/
void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/* USER CODE BEGIN 1 */



/**
* @brief Function created for refreshing the display.
*(Refreshes the display, using the functions defined in "heads_up_display.h" to get the required digits and segments.
*/
void refresh_display(void){
	// The float value to be displayed.
	extern float displayed_value;
	// Which digit is currently active.
	static uint8_t currently_active_digit = 0;
	
	// The resulting segments.
	uint8_t segments[3];
	get_segments_for_float(displayed_value, segments);
	
	GPIOD->ODR = (GPIOD->ODR & 0xFFFFFF00) | segments[currently_active_digit];
	
	switch(currently_active_digit){
		case 0:
			SET_PIN(DIGITS_0);
			RESET_PIN(DIGITS_1);
			RESET_PIN(DIGITS_2);
			break;
		case 1:
			RESET_PIN(DIGITS_0);
			SET_PIN(DIGITS_1);
			RESET_PIN(DIGITS_2);
			break;
		case 2:
			HAL_GPIO_WritePin(SEG_H_GPIO_Port, SEG_H_Pin, GPIO_PIN_SET);
			RESET_PIN(DIGITS_0);
			RESET_PIN(DIGITS_1);
			SET_PIN(DIGITS_2);
			break;
	}	
	currently_active_digit++;
	currently_active_digit %= 3;
}



/** @brief Called to check wether one key on the keypad is pressed down.
*
* IDEA: Set one column high, and check if any rows are high. If so, the button at the crossing is pressed.
*/
void check_for_digit_press(){
	static const uint32_t rows[] = { ROW_0_Pin, ROW_1_Pin, ROW_2_Pin, ROW_3_Pin };
	static const uint32_t columns[] = { COL_0_Pin, COL_1_Pin, COL_2_Pin };
	static uint8_t current_row;
	static uint8_t column;
	static char chosen_char = NULL;
	// The character that is being pushed in the keypad.
	char new_char = NULL;

	// Reset all columns, and set the current column to HIGH.
	for(int i=0; i<ROWS; i++){
		HAL_GPIO_WritePin(GPIOB, rows[i], (i == current_row) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
	
	// Read each row. If a row is high, we found the digit.
	for(column = 0; column < COLS; column++){
		// TODO: figure out why the ReadPin returns PIN_RESET when the pin is HIGH, and PIN_SET when pin is LOW.
		if(HAL_GPIO_ReadPin(GPIOB, columns[column]) == GPIO_PIN_SET){
			printf("KEY (%u, %u) is ON.\n", current_row, column);
			new_char = Keys[current_row][column];
			break;
		}else{
			printf("KEY (%u, %u) is OFF.\n", current_row, column);
		}
	}
	
	if((new_char != NULL) && (new_char != chosen_char)){
		chosen_char = new_char;
		new_keypad_value(chosen_char);
	}
	
	
	current_row++;
	current_row %= ROWS;
}



/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
