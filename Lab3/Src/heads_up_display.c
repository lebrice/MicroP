#include "heads_up_display.h"
#include "math.h"

bool display_on = false;

uint8_t display_mode = DISPLAY_RMS;
float displayed_value = 0.f;

// Easy way of converting from a digit to a segment.
const int digit_to_segments[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};


void split_three_digits(float value, int digits[3]){
	int int_value = (value * 100);
	digits[2] = int_value / 100;
	int_value -= digits[2] * 100;
	digits[1] = int_value / 10;
	int_value -= digits[1] * 10;
	digits[0] = int_value % 10;
//	printf("%f gets translated to %u, %u, %u\n", value, digits[2], digits[1], digits[0]);
}

void get_segments_for_float(float value, uint8_t segments[3]){
	int digits[3];
	split_three_digits(value, digits);
//	segments[2] = digit_to_segments[digits[2]];
//	segments[1] = digit_to_segments[digits[1]];
//	segments[0] = digit_to_segments[digits[0]];
	
	
	for(int i = 0; i < 3; i++){
		segments[i] = digit_to_segments[digits[i]];
//		printf("digit: %u, Segments: %02x\n", digits[i], segments[i]);
	}
}


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
			HAL_GPIO_WritePin(SEG_H_GPIO_Port, SEG_H_Pin, GPIO_PIN_SET); // the decimal point.
			RESET_PIN(DIGITS_0);
			RESET_PIN(DIGITS_1);
			SET_PIN(DIGITS_2);
			break;
	}	
	currently_active_digit++;
	currently_active_digit %= 3;
}
