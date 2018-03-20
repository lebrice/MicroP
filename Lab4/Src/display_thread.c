#include "display_thread.h"

extern osThreadId displayTaskHandle;
float displayed_value;

uint32_t display_on = 0;

void StartDisplayTask(void const * arguments){
	// Which digit is currently active.
	static uint8_t currently_active_digit = 0;
	
	while(true){
		osSignalWait(display_on, osWaitForever);
		while(display_on){
			// while the display is on, refresh it.
			refresh_display(currently_active_digit);
			osDelay(DISPLAY_REFRESH_INTERVAL_MS);
			
			currently_active_digit++;
			currently_active_digit %= 3;
		}
		// DISPLAY IS NOW OFF!
		RESET_PIN(DIGITS_0);
		RESET_PIN(DIGITS_1);
		RESET_PIN(DIGITS_2);
	}
}


void stop_display(){
	display_on = 0;
	// TODO: not yet implemented
//	osSignalClear(displayTaskHandle, display_on);
}

void start_display(){
	display_on = 1;
	osSignalSet(displayTaskHandle, display_on);
}


/**
* @brief Function created for refreshing the display.
*(Refreshes the display, using the functions defined in "segment_display.h" to get the required digits and segments.
*/
void refresh_display(uint8_t currently_active_digit){
	
	// The resulting segments.
	uint8_t segments[3];
	get_segments_for_float(displayed_value, segments);
	
	GPIOD->ODR = (GPIOD->ODR & 0xFFFFFF00) | segments[currently_active_digit];
	
	switch(currently_active_digit){
		case 0:
			SET_PIN(DIGITS_0);
			RESET_PIN(DIGITS_1);
			RESET_PIN(DIGITS_2);
			break;
		case 1:
			RESET_PIN(DIGITS_0);
			SET_PIN(DIGITS_1);
			RESET_PIN(DIGITS_2);
			break;
		case 2:
			HAL_GPIO_WritePin(SEG_H_GPIO_Port, SEG_H_Pin, GPIO_PIN_SET);
			RESET_PIN(DIGITS_0);
			RESET_PIN(DIGITS_1);
			SET_PIN(DIGITS_2);
			break;
	}	
}
