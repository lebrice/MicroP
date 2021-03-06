
#include "fsm.h"
#include "lis3dsh.h"
#include "main.h"
#include "blinker.h"
#include <stdio.h>

// Very useful macros for setting and resetting a given pin.
#define PIN(i) i##_Pin
#define PORT(i) i##_GPIO_Port
#define SET_PIN(i) HAL_GPIO_WritePin(PORT(i), PIN(i), GPIO_PIN_SET)
#define RESET_PIN(i) HAL_GPIO_WritePin(PORT(i), PIN(i), GPIO_PIN_RESET)
#define TOGGLE_PIN(i) HAL_GPIO_TogglePin(PORT(i), PIN(i))
#define READ_PIN(i) HAL_GPIO_ReadPin(PORT(i), PIN(i))

#define MIC_BUFFER_SIZE 10000

// signal representing if the microphone data buffer is full.
static bool mic_buffer_full_signal = 0x0001;
// signal representing if the response (server -> phone -> nucleo -> this board) has been received.
static bool response_received_signal = 0x0002;

// interval between each time we check for a tap.
// The data during the whole interval is used to determine if a tap was done.
// 
const int TAP_CHECKING_INTERVAL_MS = 500;


// The task handle for the default task. (defined in freertos.c)
extern osThreadId defaultTaskHandle;


// Handle for the UART.
extern UART_HandleTypeDef huart4;

//float pitch_roll_buffer[2][1000];
extern uint32_t mic_buffer[MIC_BUFFER_SIZE];

/** Squashes an uint32_t array down to uint16_t in-place.
*
*/
void squash(uint32_t array[], int length){
	// Create two pointers, pointing at the start of the array.
	uint32_t * source = &array[0];
	uint16_t * destination = (uint16_t*) &array[0];
	for (int i=0; i<length; i++, source++, destination++){
		// We copy from source -> destination, within array.
		*destination = (uint16_t) *source;
	}
}

void exp_moving_avg_filter(float values[], int length, float alpha){
	register float previous = values[0];
	for(int c = 0; c < length; c++) {
		previous = alpha * values[c] + (1.0f-alpha) * previous;
		values[c] = previous;
	}
}

bool detected_tap(float *samples, int num_samples, float threshold) {
//	const int Z_THRESH = 30;
//		
//	float max = samples[0];
//	int c;
//	for(c = 1; c < num_samples; c++) {
//		if(samples[c] > max) {
//			max = samples[c];
//		}
//	}
//	if(max - samples[0] > Z_THRESH && max - samples[num_samples - 1] > Z_THRESH) {
//		return 1;
//	}
//	return 0;
	asm_output output;
	asm_math(samples, num_samples, &output);
	return (output.max_value - output.rms > threshold);
}

bool detect_tap(){
	const int ACC_SAMPLING_FREQ = 100;
	const int ACC_SAMPLING_PERIOD = 1000 / ACC_SAMPLING_FREQ;
	
	
	const int BUFFER_SIZE = TAP_CHECKING_INTERVAL_MS / ACC_SAMPLING_PERIOD;
	
	//float acc_x;
	//float acc_y;
	//float acc_z;
	
	//float temp_buffer[3];
	
	float buffer[BUFFER_SIZE];
	
	float temp_buffer[3];
	float acc_x, acc_y, acc_z;
	
	for (int i=0; i<BUFFER_SIZE; i++){
		LIS3DSH_ReadACC(temp_buffer);
		acc_x = (float)temp_buffer[0];
		acc_y = (float)temp_buffer[1];
		acc_z = (float)temp_buffer[2];
		
		// what we keep: z
		buffer[i] = acc_z;
		printf("X: %2.3f, Y: %2.3f, Z: %2.3f\n", acc_x, acc_y, acc_z);
		
		osDelay(ACC_SAMPLING_PERIOD);
	}
	exp_moving_avg_filter(buffer, BUFFER_SIZE, TAP_FILTER_ALPHA);
	return detected_tap(buffer, BUFFER_SIZE, TAP_THRESHOLD);
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	osSignalSet(defaultTaskHandle, mic_buffer_full_signal);
}

/** Called when the response from the Nucleo board has been received. (after sending mic data).
* 
*/
void HAL_UART_RxCompltCallback(UART_HandleTypeDef *huart){
	// we set the flag and wake up the default thread, so it can continue and blink the LED n times.
	osSignalSet(defaultTaskHandle, response_received_signal);
}
	

void single_tap(){
	// MICROPHONE-related variables.
	const int MIC_RECORDING_FREQ_HZ = 10000;
	const int MIC_RECORDING_LENGTH_SECS = 1;
	const int MIC_RECORDING_SAMPLE_COUNT = MIC_RECORDING_LENGTH_SECS * MIC_RECORDING_FREQ_HZ;
	
	extern ADC_HandleTypeDef hadc1;
	
	// Buffer holding the microphone samples.
	// TODO: DMA seems to only be able to use 32-bit wide data.
	//uint32_t mic_buffer[MIC_RECORDING_SAMPLE_COUNT];
	
	// Start recording audio using the ADC with DMA.
	
	//Wait before recording
	SET_PIN(LED_GREEN);
	osDelay(PRE_RECORDING_DELAY_MS);
	RESET_PIN(LED_GREEN);
	start_blinking();
	HAL_ADC_Start_DMA(&hadc1, mic_buffer, MIC_RECORDING_SAMPLE_COUNT);
	
	
	// Wait on the mic_buffer_full signal, which will be set by the ADC callback once the buffer has been filled.
	osSignalWait(mic_buffer_full_signal, osWaitForever);
	HAL_ADC_Stop_DMA(&hadc1);
	stop_blinking();
	
	// Set the GPIO pin to let the nucleo board known that we want to transmit some Microphone data.
	//SET_PIN(IS_MIC_DATA);
	squash(mic_buffer,MIC_RECORDING_SAMPLE_COUNT);
	// Send the mic data over UART to the Nucleo board.
	int bytes_to_send = MIC_RECORDING_SAMPLE_COUNT * sizeof(mic_buffer[0]) / sizeof(uint8_t);
	HAL_UART_Transmit(&huart4, (uint8_t*) mic_buffer, bytes_to_send, HAL_MAX_DELAY);	
	
	// Wait for the response (might take very, very long time. Therefore, we use interrupt mode.)
	uint8_t spoken_digit;
	HAL_UART_Receive_IT(&huart4, &spoken_digit, 1);
	
	// wait for the response.
	osSignalWait(response_received_signal, osWaitForever);
	
	// Blink an LED N times, with 'N' being the digit that was spoken.
	if(spoken_digit == 0)
	{
		// if N is zero, then the spoken digit was not recognized. 
		// We shine a RED LED for 3 seconds.
		SET_PIN(LED_RED);
		osDelay(3000);
		RESET_PIN(LED_RED);
	}
	else
	{
		// Flash the blue LED 'N' times.
		for (int i=0; i<spoken_digit; i++){
			SET_PIN(LED_BLUE);
			osDelay(250);
			RESET_PIN(LED_BLUE);
			osDelay(250);
		}
	}
}

float calculate_pitch(float x, float y, float z){
	const float PI = acosf(-1);
	return (atan2(x, sqrt(y*y + z*z))*180.0)/PI;
}

float calculate_roll(float x, float y, float z){
	const float PI = acosf(-1);
	return (atan2(-y, z)*180.0)/PI;
}

void double_tap(){	
	const int ACC_RECORDING_LENGTH_MS = 10000;
	const int ACC_RECORDING_FREQ = 100;
	const int ACC_SAMPLING_PERIOD_MS = 1000 / ACC_RECORDING_FREQ;
	
	
	// TODO: implement these functions.
	extern float calculate_pitch(float, float, float);
	extern float calculate_roll(float, float, float);
	
	const int ACC_RECORDING_SAMPLE_COUNT = ACC_RECORDING_LENGTH_MS / ACC_SAMPLING_PERIOD_MS;
	
//	float acc_buffer[3][ACC_RECORDING_SAMPLE_COUNT];
	
	static float pitch_roll_buffer[2][ACC_RECORDING_SAMPLE_COUNT];
	float temp_buffer[3];
	float acc_x, acc_y, acc_z;
	float pitch, roll;
	
	//Wait before recording
	SET_PIN(LED_GREEN);
	osDelay(PRE_RECORDING_DELAY_MS);
	RESET_PIN(LED_GREEN);
	
	start_blinking();
	
	printf("ACC COUNT: %d\n",ACC_RECORDING_SAMPLE_COUNT);
	// Record the accelerometer data for 10 seconds.
	for(int i=0; i< ACC_RECORDING_SAMPLE_COUNT; i++){
		// Read a sample from the Accelerometer via SPI.
		printf("i = %d\n",i);
		LIS3DSH_ReadACC(temp_buffer);
		acc_x = (float)temp_buffer[0];
		acc_y = (float)temp_buffer[1];
		acc_z = (float)temp_buffer[2];
		
		pitch = calculate_pitch(acc_x, acc_y, acc_z);
		roll = calculate_roll(acc_x, acc_y, acc_z);
		pitch_roll_buffer[0][i] = pitch;
		pitch_roll_buffer[1][i] = roll;
		
		osDelay(ACC_SAMPLING_PERIOD_MS);
	}
	
	stop_blinking();

	// Send the data via UART to the nucleo board.
	// Send the Pitch array over to the Nucleo board via UART.
	exp_moving_avg_filter(pitch_roll_buffer[0], ACC_RECORDING_SAMPLE_COUNT, ACC_FILTER_ALPHA);
	exp_moving_avg_filter(pitch_roll_buffer[1], ACC_RECORDING_SAMPLE_COUNT, ACC_FILTER_ALPHA);
	int bytes_to_send = ACC_RECORDING_SAMPLE_COUNT * sizeof(float) / sizeof(uint8_t);
	HAL_UART_Transmit(&huart4, (uint8_t*) pitch_roll_buffer[0], bytes_to_send, HAL_MAX_DELAY);

	// Wait for just a little bit, giving time for the Nucleo board to receive the data. (might not be really necessary.)
	osDelay(10);

	// Send the Roll array over.
	HAL_UART_Transmit(&huart4, (uint8_t*) pitch_roll_buffer[1], bytes_to_send, HAL_MAX_DELAY);	
	
	printf("Done with double-tap.\n");
}




/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{
	//uint8_t data[3] = {1,2,3};
	static STATE state;
	osDelay(10); //Allow accelerometer data to initialize to escape first accelerometer differential
	detect_tap(); //Escape initial large change in acceleration to avoid detecting single tap immediately
  while(1){
		switch(state)
		{
				case IDLE:
					if(detect_tap()){
						state = TAP_RECENT;
					}
					break;
				
				case TAP_RECENT:
					// if we detect another tap, go to doubletap, else, to singletap.
					// TODO: Possibly configure the detect_tap to take in a customizable interval between two consecutive taps
					// What about something like count_taps(interval), which returns the number of local maxima in the interval? 
					HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
					state = (detect_tap()) ? DOUBLETAP : SINGLETAP;
					break;
				
				case SINGLETAP:
					printf("Single Tap!\n");
					HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
					HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
				
					SET_PIN(IS_MIC_DATA);
					osDelay(1);
					SET_PIN(DATA_INTERRUPT);
					osDelay(1);
					RESET_PIN(DATA_INTERRUPT);				
				
					//HAL_UART_Transmit(&huart4, data, 3, 100);
					single_tap();
					// We're done. return to the IDLE state.
					HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
					state = IDLE;				
					break;

				case DOUBLETAP:
					printf("Double Tap!\n");
					HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
					HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
				
					RESET_PIN(IS_MIC_DATA);
					osDelay(1);
					SET_PIN(DATA_INTERRUPT);
					osDelay(1);
					RESET_PIN(DATA_INTERRUPT);	
				
					double_tap();
					// We're done. return to the IDLE state.
					HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
					state = IDLE;
					break;

				default:
					break;
		}		
	}
}

