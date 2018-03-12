#ifndef __adc_thread_h
#define __adc_thread_h
#endif

#ifndef __stdint_h
#include <stdint.h>
#endif

#define ADC_BUFFER_SIZE 50





extern float filtered_ADCBuffer[ADC_BUFFER_SIZE];

// Buffer that holds Unfiltered data populated with DMA. 
extern uint32_t ADCBufferDMA[ADC_BUFFER_SIZE];

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



void adc_buffer_full_callback(void);
void FIR_C(int Input, float* Output);
void asm_math(float *inputValues, int size, asm_output *results);


float DigitalToAnalogValue(int digital_value);
int analog_to_digital_value(float analog_voltage);
