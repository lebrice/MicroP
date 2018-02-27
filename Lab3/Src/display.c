#include "display.h"
#include <stdint.h>
#include "main.h"

/**
* @brief Function created for refreshing the display.
*(Refreshes the display, using the functions defined in "heads_up_display.h" to get the required digits and segments.
*/
void refresh_display(void){
	
	// The float value to be displayed.
	extern float displayed_value;
		
	// Which digit is currently active.
	static uint8_t currently_active_digit = 0;
	
	
	
	// The resulting segments.
	uint8_t segments[3];
	if (!display_on){
		RESET_PIN(DIGITS_0);
		RESET_PIN(DIGITS_1);
		RESET_PIN(DIGITS_2);
		return;
	}
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
	currently_active_digit++;
	currently_active_digit %= 3;
}


