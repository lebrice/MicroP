#define ADC

#ifndef __stdint_h
#include <stdint.h>
#endif



const int ADC_BUFFER_SIZE = 8400;

extern float filtered_ADCBuffer[ADC_BUFFER_SIZE];

// Buffer that holds Unfiltered data populated with DMA. 
extern uint32_t ADCBufferDMA[ADC_BUFFER_SIZE];


void adc_buffer_full_callback(void);
