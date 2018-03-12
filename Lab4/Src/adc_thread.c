#include "adc_thread.h"



extern osThreadId adcTaskHandle;
extern const int PWM_TIMER_PERIOD;

extern ADC_HandleTypeDef hadc1;


void start_adc(void);
void stop_adc(void);


float filtered_ADCBuffer[ADC_BUFFER_SIZE];

// Buffer that holds Unfiltered data populated with DMA. 
uint32_t ADCBufferDMA[ADC_BUFFER_SIZE];

static PastResultsVector past_ten_seconds_results;

void StartAdcTask(void const * arguments){
	extern bool adc_buffer_full;
	start_adc();
	// wait for the buffer to be full.
	osSignalWait(adc_buffer_full, 0);
	adc_buffer_full_callback();
	
	// not yet implemented:
	//osSignalClear(adcTaskHandle, adc_buffer_full);
	adc_buffer_full = false;
	
	stop_adc();	
}



void start_adc(){
	HAL_ADC_Start_DMA(&hadc1, ADCBufferDMA, ADC_BUFFER_SIZE);
}

void stop_adc(){
	HAL_ADC_Stop_DMA(&hadc1);
}


/** @brief This function is called when the ADC buffer is full.
*
*/
void adc_buffer_full_callback()
{
	extern float displayed_value;
	
	asm_output last_results;
	float current_rms_voltage;
	
	// use the filter on each value in the raw buffer.
	for(int i=0; i < ADC_BUFFER_SIZE; i++){
		FIR_C(ADCBufferDMA[i], &filtered_ADCBuffer[i]);
	}
	
	asm_math(filtered_ADCBuffer, ADC_BUFFER_SIZE, &last_results);
	current_rms_voltage = DigitalToAnalogValue(last_results.rms);
	
	// TODO: use round to display the closest value.
	displayed_value = current_rms_voltage;
	adjust_duty_cycle(current_rms_voltage);
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

void FIR_C(int Input, float* Output){
	//Idea: use the buffer like a circular queue.
	//- Add the element to the buffer, using the head pointer.
	//- Update tail accordingly
	//- iterate in the buffer, going from head to tail, and add up the results
	
	// Array of weights
	static const int FILTER_ORDER = 5;
	static const float WEIGHT = 1.f / FILTER_ORDER;
	
	static float weights[FILTER_ORDER];
		
	if(weights[0] == 0.f){
		for(int i=0; i<FILTER_ORDER; i++)
			weights[i] = WEIGHT;
	}
		
		
	// Buffer that will hold the values as they come in.
	static int buffer[FILTER_ORDER];
	static int head, tail = 0;
	
	int i;
	float result = 0.f;
	head = (head + 1) % FILTER_ORDER; // Update the head.
	if(head == tail){ // Update the tail, if necessary.
		tail = (tail + 1) % FILTER_ORDER;
	}
	buffer[head] = Input; // write the new value in.
	for(i=0; i<FILTER_ORDER; i++){
		// move backward from 'head' to 'tail', adding up the values.
		result += weights[i] * buffer[(head - i + FILTER_ORDER) % FILTER_ORDER];
	}
	*Output = result; // place the result at the given location.
}


/** @brief Set the period of the PWM timer
* @param new_period: new timer period.
*/
void pwm_duty_cycle(uint16_t new_period) //input percentage
{
		extern TIM_HandleTypeDef htim3;
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



float DigitalToAnalogValue(int digital_value){
	return 3.0*(digital_value)/4095;
}

int analog_to_digital_value(float analog_voltage){
	return (analog_voltage / 3.0f) * 4095;
}


