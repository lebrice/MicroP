// keypad.c
#include "keypad.h"

static float input_value = 0.0f;

float make_float_from_last_two_digits(uint8_t digits[2]);

/** @bried Called whenever a new keypad value is received.
* IDEA: state machine:
* INIT --(digit entered)--> one digit
* one_digit --(digit entered)--> Two digits
* two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
* two_digits --(pound sign)--> INIT
*/
void new_keypad_value(uint8_t new_keypad_value){
	static uint8_t digits[2];
	switch(new_keypad_value){
		case KEYPAD_POUND:
			// We use the last two digits to assign the input value.
			input_value = make_float_from_last_two_digits(digits);
			break;
		case KEYPAD_STAR:
			// We want to reset the digits.
			digits[0] = 0;
			digits[1] = 0;
			break;
		case KEYPAD_0:
			digits[1] = digits[0];
			digits[0] = 0;
			break;
		case KEYPAD_1:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_2:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_3:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_4:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_5:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_6:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_7:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_8:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case KEYPAD_9:
			digits[1] = digits[0];
			digits[0] = 1;
			break;
	}
	
	
}

void add_new_digit(uint8_t new_digit, uint8_t digits[2]){
	digits[1] = digits[0];
	digits[0] = new_digit;
}

float make_float_from_last_two_digits(uint8_t digits[2]){
	// TODO:
	return 0.f;
}
