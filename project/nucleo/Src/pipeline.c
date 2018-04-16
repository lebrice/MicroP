#include "pipeline.h"
#include "main.h"
#include "custom_service.h"



extern UART_HandleTypeDef huart2;
extern volatile uint8_t set_connectable;
extern volatile int connected;

// Bluetooth Service Handle
extern uint16_t custom_service_handle;
// Bluetooth Service Characteristic Handles
extern uint16_t acc_char_handle, mic_char_handle, digit_char_handle;

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
		printf("Received Sample of  Microphone data!\n");
		
		uint16_t offset = 0;
		
		for(int i = 0; i < MIC_BATCH_COUNT; i++) {
			for(int j = 0; j < MIC_SAMPLES_PER_BATCH; j++) {
				mic_batch.data[j] = mic_buffer.data[j + offset];
			}
			offset += MIC_SAMPLES_PER_BATCH;
			// Send the batch via BLE

			printf("Sending MIC Batch #%d / %d\n", i, MIC_BATCH_COUNT);
			mic_update(&mic_batch);
			
			
			
			// wait for a bit
			HAL_Delay(BLE_DELAY_BETWEEN_PACKETS_MS);
		}
	}else{
		// Receive pitch
		uint16_t bytes_to_receive = ACC_SAMPLE_COUNT * ACC_SAMPLE_BYTES;
		HAL_UART_Receive(&huart2, (uint8_t*) &acc_buffer.pitch, bytes_to_receive, HAL_MAX_DELAY);
		printf("Received PITCH!\n");
		// Receive roll
		HAL_UART_Receive(&huart2, (uint8_t*) &acc_buffer.roll, bytes_to_receive, HAL_MAX_DELAY);
		printf("Received ROLL!\n");
		int offset = 0;
		for (int i=0; i<ACC_BATCH_COUNT; i++, offset += ACC_SAMPLES_PER_BATCH){
			// Copy over the data.
			for(int j=0; j<ACC_SAMPLES_PER_BATCH; j++){
				acc_batch.pitch[j] = acc_buffer.pitch[offset + j];
				acc_batch.roll[j] = acc_buffer.roll[offset + j];
			}
			printf("Sending ACC Batch #%u / %u\n", i, ACC_BATCH_COUNT);
			// Send over a batch of data.
			acc_update(&acc_batch);
			
			// Wait for a certain time, 
			HAL_Delay(BLE_DELAY_BETWEEN_PACKETS_MS);
		}
	}
}




/** 
 * @brief  This function is called when the digit comes back (when the phone changes the 'digit' char.)
 * @param  Handle of the attribute
 * @param  Size of the modified attribute data
 * @param  Pointer to the modified attribute data
 * @retval None
 */
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data)
{
	printf("GATT attribute %x was modified!\n", handle);
	/* If GATT client has modified 'LED button characteristic' value, toggle LED2 */
	if(handle == digit_char_handle + 1){
		printf("Received data:\t'");
		for(int i=0; i<data_length; i++){
			printf("%c", att_data[i]);
		}
		printf("'\n");
		HAL_UART_Transmit(&huart2, att_data, data_length, HAL_MAX_DELAY);
		printf("Done sending result over UART\n");
	}
}
	
