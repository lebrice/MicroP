
#include "fsm.h"
#include "lis3dsh.h"
#include "main.h"

// Very useful macros for setting and resetting a given pin.
#define PIN(i) i##_Pin
#define SET_PIN(i) HAL_GPIO_WritePin(i##_GPIO_Port, PIN(i), GPIO_PIN_SET)
#define RESET_PIN(i) HAL_GPIO_WritePin(i##_GPIO_Port, PIN(i), GPIO_PIN_RESET)
#define TOGGLE_PIN(i) HAL_GPIO_TogglePin(i##_GPIO_Port, PIN(i))

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

float temp_buffer[3];
float acc_x, acc_y, acc_z;

extern uint32_t mic_buffer[MIC_BUFFER_SIZE];


bool detected_tap(float *samples, int num_samples) {
	const int Z_THRESH = 30;
		
	float max = samples[0];
	int c;
	for(c = 1; c < num_samples; c++) {
		if(samples[c] > max) {
			max = samples[c];
		}
	}
	if(max - samples[0] > Z_THRESH && max - samples[num_samples - 1] > Z_THRESH) {
		return 1;
	}
	return 0;
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
	
	
	for (int i=0; i<BUFFER_SIZE; i++){
		LIS3DSH_ReadACC(&temp_buffer[0]);
		acc_x = (float)temp_buffer[0];
		acc_y = (float)temp_buffer[1];
		acc_z = (float)temp_buffer[2];
		
		// what we keep: z
		buffer[i] = acc_z;
		
		osDelay(ACC_SAMPLING_PERIOD);
	}
	
	return detected_tap(buffer, BUFFER_SIZE);
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
	const int MIC_RECORDING_FREQ_HZ = 16000;
	const int MIC_RECORDING_LENGTH_SECS = 1;
	const int MIC_RECORDING_SAMPLE_COUNT = MIC_RECORDING_LENGTH_SECS * MIC_RECORDING_FREQ_HZ;
	
	extern ADC_HandleTypeDef hadc1;
	
	// Buffer holding the microphone samples.
	// TODO: DMA seems to only be able to use 32-bit wide data.
	//uint32_t mic_buffer[MIC_RECORDING_SAMPLE_COUNT];
	
	// Start recording audio using the ADC with DMA.
	HAL_ADC_Start_DMA(&hadc1, mic_buffer, MIC_RECORDING_SAMPLE_COUNT);
	
	
	// Wait on the mic_buffer_full signal, which will be set by the ADC callback once the buffer has been filled.
	osSignalWait(mic_buffer_full_signal, osWaitForever);
	HAL_ADC_Stop_DMA(&hadc1);
	
	
	// Set the GPIO pin to let the nucleo board known that we want to transmit some Microphone data.
	SET_PIN(IS_MIC_DATA);
	
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
	
	// TODO: decide if we send pitch and roll, or the raw data.
//	float acc_buffer[3][ACC_RECORDING_SAMPLE_COUNT];
	float pitch_roll_buffer[2][ACC_RECORDING_SAMPLE_COUNT];
	
	float temp_buffer[3];
	float acc_x, acc_y, acc_z;
	float pitch, roll;
	
	
	
	// Record the accelerometer data for 10 seconds.
	for(int i=0; i< ACC_RECORDING_SAMPLE_COUNT; i++){
		// Read a sample from the Accelerometer via SPI.
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

	// Send the data via UART to the nucleo board.

	// Set the IS_MIC_DATA GPIO pin to '0', to tell the Nucleo Board that we are sending accelerometer data.					
	RESET_PIN(IS_MIC_DATA);



	// Send the Pitch array over to the Nucleo board via UART.
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
					state = (detect_tap()) ? DOUBLETAP : SINGLETAP;
					break;
				
				case SINGLETAP:
					printf("Single Tap!\n");
					HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
					SET_PIN(DATA_INTERRUPT);
					osDelay(1);
					RESET_PIN(DATA_INTERRUPT);
					single_tap();
					// We're done. return to the IDLE state.
					state = IDLE;				
					break;

				case DOUBLETAP:
					printf("Double Tap!\n");
					HAL_GPIO_TogglePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin);
					double_tap();
					// We're done. return to the IDLE state.
					state = IDLE;
					break;

				default:
					break;
		}		
	}
}

