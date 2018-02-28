/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>

#include <stdbool.h>
#ifndef DISPLAY_RMS
#include "heads_up_display.h"
#endif

#ifndef FSM
#include "fsm.h"
#endif

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

const int ADC_BUFFER_SIZE = 50;
const int PWM_TIMER_PERIOD = 100;


float filtered_ADCBuffer[ADC_BUFFER_SIZE];

// Buffer that holds Unfiltered data populated with DMA. 
static uint32_t ADCBufferDMA[ADC_BUFFER_SIZE];




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
typedef struct {
	float last_RMS;
	float past_mins[10];
	float past_maxs[10];	
} PastResultsVector;

typedef struct {
	float rms;
	float max_value;
	float min_value;
	int max_index;
	int min_index;	
} asm_output;


// Called in order to adjust the duty cycle.
void adjust_duty_cycle(float current_rms);

void start_adc(void);
void stop_adc(void);


// Function that is called whenever the blue button is pressed.
void button_pressed_callback(void);
void adc_buffer_full_callback(void);
void FIR_C(int Input, float* Output);
void asm_math(float *inputValues, int size, asm_output *results);
void pwm_duty_cycle(float percentage);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/** @brief Set the period of the PWM timer
* @param percentage: float value from 0 to 1.
*/
void pwm_duty_cycle(float percentage) //input percentage
{
    uint16_t value = (uint16_t)(PWM_TIMER_PERIOD)*percentage; //(period)*(percent/100)
		TIM_OC_InitTypeDef sConfigOC;
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = value;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  
}

void start_adc(){
	HAL_ADC_Start_DMA(&hadc1, ADCBufferDMA, ADC_BUFFER_SIZE);
}

void stop_adc(){
	HAL_ADC_Stop_DMA(&hadc1);
}

/** @brief Controller which adjusts the PWM duty cycle in order to match the current target RMS voltage.
* @param current_rms: The current RMS voltage from the ADC.
*/
void adjust_duty_cycle(float current_rms){ 
	extern float target_voltage;
	
	// a damping constant, that limits the rate of change of the percentage.
	static const float damping = 0.1f;
	
	
	static float old_voltage;
	// TODO: check if we need to set this to some initial value.
	static float old_percentage;
	static float current_percentage = 0.001f;
	
	static float current_voltage;
	static float d_v;
	static float d_p;
	
	static float difference;
	current_voltage = current_rms;
	
	d_v = current_voltage - old_voltage; 
	d_p = current_percentage - old_percentage;
	
	difference = current_voltage - target_voltage;
	
	// avoid possible divide-by-zero errors in below equation.
	d_v = (-0.001f < d_v <= 0) ? -0.001f : d_v;
	d_v = (0 < d_v < 0.001f) ? 0.001f: d_v;	
	
	/** @brief This is the main controller logic.
	*
	* IDEA: adjust the percentage by using the derivative of duty cycle with respect to change in voltage.
	* new_p = old_p - k * difference * (d_p / d_v);
	*/
	current_percentage = old_percentage - damping * difference * (d_p / d_v);
	
	// The percentage is limited between 0% and 100%.
	current_percentage = BOUND(current_percentage, 0.0f, 1.0f);
	printf("Current voltage: %2.3f: Current Percentage: %2.3f\n", current_voltage, current_percentage);


//	if(diff > 0){
//		current_percentage -= 0.001f;
//	}else{
//		current_percentage += 0.001f;
//	}

	old_percentage = current_percentage;
		
	pwm_duty_cycle(current_percentage);
}


static PastResultsVector past_ten_seconds_results;

float DigitalToAnalogValue(int digital_value){
	float AnalogVal = 3.0*(digital_value)/4095;
	return AnalogVal;
}

int analog_to_digital_value(float analog_voltage){
	return (analog_voltage / 3.0f) * 4095;
}

void find_min_max_last_10_secs(asm_output last_results, float results[2]){
	static int head;
	static int tail;
	
	int current;
	float temp_max;
	float temp_min;
	float min_last_10_secs;
	float max_last_10_secs;
	
	
	float min, max, rms;
	
	min = last_results.min_value;
	max = last_results.max_value;
	rms = last_results.rms;
	
	
	head = (head + 1) % 10; // Update the head.
	if(head == tail){ // Update the tail, if necessary.
		tail = (tail + 1) % 10; 
	}
	past_ten_seconds_results.past_mins[head] = min; // write the new value in.
	past_ten_seconds_results.past_maxs[head] = max;
	past_ten_seconds_results.last_RMS = rms;
	
	//We have to calculate the min and max of the last 10 seconds.
	current = tail;
	min_last_10_secs = min;
	max_last_10_secs = max;
	
	
	// Loop through the array of past results, and find the min and max.
	while(current != head){
		// Update the Min.
		temp_min = past_ten_seconds_results.past_mins[current];
		min_last_10_secs = (temp_min < min_last_10_secs && temp_min != 0)? temp_min : min_last_10_secs;
		
		// Update the Max.
		temp_max = past_ten_seconds_results.past_maxs[current];
		max_last_10_secs = (temp_max > max_last_10_secs)? temp_max : max_last_10_secs;
		current = (current + 1) % 10;
	}
	
	results[0] = min_last_10_secs;
	results[1] = max_last_10_secs;	
}


/** @brief This function is called when the ADC buffer is full.
*
*/
void adc_buffer_full_callback()
{
	extern float displayed_value;
	
	asm_output last_results;
	float current_rms_voltage;
	
	asm_math(filtered_ADCBuffer, ADC_BUFFER_SIZE, &last_results);
	current_rms_voltage = DigitalToAnalogValue(last_results.rms);
	
	displayed_value = current_rms_voltage;
	adjust_duty_cycle(current_rms_voltage);
}

void FIR_C(int Input, float* Output){
	//Idea: use the buffer like a circular queue.
	//- Add the element to the buffer, using the head pointer.
	//- Update tail accordingly
	//- iterate in the buffer, going from head to tail, and add up the results
	
	// Array of weights
	static float weights[5] = {0.2, 0.2, 0.2, 0.2, 0.2};
	// Buffer that will hold the values as they come in.
	static int buffer[5];
	static int head, tail = 0;
	
	int i;
	float result = 0.f;
	head = (head + 1) % 5; // Update the head.
	if(head == tail){ // Update the tail, if necessary.
		tail = (tail + 1) % 5;
	}
	buffer[head] = Input; // write the new value in.
	for(i=0; i<5; i++){
		// move backward from 'head' to 'tail', adding up the values.
		result += weights[i] * buffer[(head - i + 5) % 5];
	}
	*Output = result; // place the result at the given location.
}

// Called when the buffer is filled.
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	if(AdcHandle->Instance == ADC1){
		// use the filter on each value in the raw buffer.
		for(int i=0; i < ADC_BUFFER_SIZE; i++){
			FIR_C(ADCBufferDMA[i], &filtered_ADCBuffer[i]); 
		}
		adc_buffer_full_callback();
		HAL_ADC_Stop_DMA(&hadc1);
		HAL_ADC_Start_DMA(&hadc1, ADCBufferDMA, ADC_BUFFER_SIZE);
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


//	
//	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2048);
//	HAL_DAC_Start(&hdac, DAC_CHANNEL_1); 
	
  

//	SET_PIN(DIGITS_0);
//	HAL_GPIO_WritePin(SEG_G_GPIO_Port, SEG_G_Pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
	
	
	
	// Start the timers.
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3); 
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	
  HAL_TIM_Base_Start_DMA(&htim2, ADCBufferDMA, ADC_BUFFER_SIZE);
	
	sleep();
//	HAL_ADC_Start_DMA(&hadc1,ADCBufferDMA, ADC_BUFFER_SIZE);
	//pwm_duty_cycle(50);
	
	
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
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* DAC init function */
static void MX_DAC_Init(void)
{

  DAC_ChannelConfTypeDef sConfig;

    /**DAC Initialization 
    */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**DAC channel OUT1 config 
    */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 200; // (we want 5kHz.)
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000; // (1 KHz)
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

	sConfigOC.Pulse = 0;
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
	
  HAL_TIM_MspPostInit(&htim3);

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, DIGITS_2_Pin|DIGITS_3_Pin|DIGITS_4_Pin|DIGITS_0_Pin 
                          |DIGITS_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, ROW_1_Pin|ROW_2_Pin|ROW_3_Pin|ROW_0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED3_Pin|LED4_Pin|LD5_Pin|LD6_Pin 
                          |SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin 
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin|SEG_H_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DIGITS_2_Pin DIGITS_3_Pin DIGITS_4_Pin DIGITS_0_Pin 
                           DIGITS_1_Pin */
  GPIO_InitStruct.Pin = DIGITS_2_Pin|DIGITS_3_Pin|DIGITS_4_Pin|DIGITS_0_Pin 
                          |DIGITS_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : BLUE_BUTTON_Pin */
  GPIO_InitStruct.Pin = BLUE_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLUE_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ROW_1_Pin ROW_2_Pin ROW_3_Pin ROW_0_Pin */
  GPIO_InitStruct.Pin = ROW_1_Pin|ROW_2_Pin|ROW_3_Pin|ROW_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : COL_0_Pin COL_1_Pin COL_2_Pin */
  GPIO_InitStruct.Pin = COL_0_Pin|COL_1_Pin|COL_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LED3_Pin LED4_Pin LD5_Pin LD6_Pin */
  GPIO_InitStruct.Pin = LED3_Pin|LED4_Pin|LD5_Pin|LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG_A_Pin SEG_B_Pin SEG_C_Pin SEG_D_Pin 
                           SEG_E_Pin SEG_F_Pin SEG_G_Pin SEG_H_Pin */
  GPIO_InitStruct.Pin = SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin 
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin|SEG_H_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
