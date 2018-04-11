#include "adc.h"

float filtered_ADCBuffer[ADC_BUFFER_SIZE];

// Buffer that holds Unfiltered data populated with DMA. 
uint32_t ADCBufferDMA[ADC_BUFFER_SIZE];

/** @brief This function is called when the ADC buffer is full.
*
*/
void adc_buffer_full_callback()
{
	//TODO: need to send ADC data when called...	

}

