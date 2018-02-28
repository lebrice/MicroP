#include "adc.h"

float filtered_ADCBuffer[ADC_BUFFER_SIZE];

// Buffer that holds Unfiltered data populated with DMA. 
uint32_t ADCBufferDMA[ADC_BUFFER_SIZE];

static PastResultsVector past_ten_seconds_results;


/** @brief This function is called when the ADC buffer is full.
*
*/
void adc_buffer_full_callback()
{
	extern void adjust_duty_cycle(float);
	extern float displayed_value;
	
	asm_output last_results;
	float current_rms_voltage;
	
	// use the filter on each value in the raw buffer.
	for(int i=0; i < ADC_BUFFER_SIZE; i++){
		FIR_C(ADCBufferDMA[i], &filtered_ADCBuffer[i]);
	}
	
	
	asm_math(filtered_ADCBuffer, ADC_BUFFER_SIZE, &last_results);
	current_rms_voltage = DigitalToAnalogValue(last_results.rms);
	
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


float DigitalToAnalogValue(int digital_value){
	return 3.0*(digital_value)/4095;
}

int analog_to_digital_value(float analog_voltage){
	return (analog_voltage / 3.0f) * 4095;
}


