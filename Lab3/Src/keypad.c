// keypad.c
#include "keypad.h"

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __stdio_h
#include <stdio.h>
#endif

// The value that is to be matched by the 
static float target_value = 0.0f;

float make_float_from_last_three_digits(uint8_t digits[3]);

bool is_valid_target_value(float target_value);


/** @bried Called whenever a new keypad value is received.
* IDEA: state machine:
* INIT --(digit entered)--> one digit
* one_digit --(digit entered)--> Two digits
* two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
* two_digits --(pound sign)--> INIT
*/
void new_keypad_value(char new_keypad_value){
	static float temp_value;
	
	extern float displayed_value;
	static uint8_t digits[3];
	
	float new_target;
	
	switch(new_keypad_value){
		case '#':
			// We use the last two digits to assign the input value.
			new_target = make_float_from_last_three_digits(digits);
			if (is_valid_target_value(new_target)){
				target_value = new_target;
			}
			// TODO: show the ADC value instead!
			displayed_value = target_value;
			break;
		case '*':
			// We want to remove the last digit.
		  digits[0] = digits[1];
			digits[1] = digits[2];
			digits[2] = 0;
			break;
		default:
			digits[2] = digits[1];
			digits[1] = digits[0];
			digits[0] = (int)(new_keypad_value - '0');
	}
	printf("digits: %u, %u, %u, \n", digits[2], digits[1], digits[0]);
	temp_value = make_float_from_last_three_digits(digits);
	
	//TODO: check that this makes sense with the FSM diagram. 
	displayed_value = temp_value;
}

bool is_valid_target_value(float target_value){
	return (target_value > 0.f) && (target_value < 3.0f);
}



float make_float_from_last_three_digits(uint8_t digits[3]){
	// TODO:
	float value = 1.0f * digits[2] + 0.1f * digits[1] + 0.01f * digits[0];
	return value;
}
