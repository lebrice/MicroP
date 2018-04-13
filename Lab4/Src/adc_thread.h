#ifndef __adc_thread_h
#define __adc_thread_h
#endif

#ifndef __stdint_h
#include <stdint.h>
#endif

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __CMSIS_OS_H
#include "cmsis_os.h"
#endif

#ifndef __stm32f4xx_hal_h
#include "stm32f4xx_hal.h"
#endif

#ifndef __math_h
#include "math.h"
#endif


#define ABS(x) ((x < 0)? -x : x)
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define BOUND(x, lower, upper) (MAX(MIN(x, upper), lower))

#define ADC_BUFFER_SIZE 8400


void StartAdcTask(void const * arguments);


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
void adjust_duty_cycle(float);
void set_pwm_duty_cycle(uint16_t);

void FIR_C(int Input, float* Output);
void asm_math(float *inputValues, int size, asm_output *results);


float DigitalToAnalogValue(int digital_value);
int analog_to_digital_value(float analog_voltage);
