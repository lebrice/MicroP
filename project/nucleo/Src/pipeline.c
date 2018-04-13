#include "pipeline.h"
#include "main.h"
#include "custom_service.h"



extern UART_HandleTypeDef huart2;
extern volatile uint8_t set_connectable;
extern volatile int connected;

// Bluetooth Service Handle
extern uint16_t custom_service_handle;
// Bluetooth Service Characteristic Handles
extern uint16_t pitch_roll_char_handle, voice_char_handle;

/** THE ENTIRE PIPELINE.
*
*/
void pipeline(void){
	static AccBuffer acc_buffer;
	static MicBuffer mic_buffer;
	
	// Temp variables which hold one batch's worth of data.
	AccBatch acc_batch;
	MicBatch mic_batch;
	
	uint8_t is_mic_data = READ_PIN(IS_MIC_DATA);		
	
	if(is_mic_data){
		// Receive the Microphone data via UART.
		HAL_UART_Receive(&huart2, (uint8_t*) &mic_buffer.data, MIC_TOTAL_BYTES, HAL_MAX_DELAY);
		uint16_t offset = 0;
		for(int i = 0; i < MIC_BATCH_COUNT; ++i) {
			offset = i * MIC_SAMPLES_PER_BATCH;
			for(int j = 0; j < MIC_SAMPLES_PER_BATCH; ++j) {
				mic_batch.data[j] = mic_buffer.data[j + offset];
			}
			
			// Send the batch via BLE
			mic_update(&mic_batch);
			
			// wait for a bit
			HAL_Delay(BLE_DELAY_BETWEEN_PACKETS_MS);
		}
	}else{
		// Receive pitch
		uint16_t bytes_to_receive = ACC_SAMPLE_COUNT * ACC_SAMPLE_BYTES;
		HAL_UART_Receive(&huart2, (uint8_t*) &acc_buffer.pitch, bytes_to_receive, HAL_MAX_DELAY);
		
		// Receive roll
		HAL_UART_Receive(&huart2, (uint8_t*) &acc_buffer.roll, bytes_to_receive, HAL_MAX_DELAY);
		
		int offset = 0;
		for (int i=0; i<ACC_BATCH_COUNT; i++, offset += ACC_SAMPLES_PER_BATCH){
			// Copy over the data.
			for(int j=0; j<ACC_SAMPLES_PER_BATCH; j++){
				acc_batch.pitch[j] = acc_buffer.pitch[offset + j];
				acc_batch.roll[j] = acc_buffer.roll[offset + j];
			}
			
			// Send over a batch of data.
			acc_update(&acc_batch);
			
			// Wait for a certain time, 
			HAL_Delay(BLE_DELAY_BETWEEN_PACKETS_MS);
		}
		
	}
		
}
	