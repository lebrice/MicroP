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
void new_keypad_value(char new_keypad_value){
	static uint8_t digits[2];
	switch(new_keypad_value){
		case '#':
			// We use the last two digits to assign the input value.
			input_value = make_float_from_last_two_digits(digits);
			break;
		case '*':
			// We want to reset the digits.
			digits[0] = 0;
			digits[1] = 0;
			break;
		case '0':
			digits[1] = digits[0];
			digits[0] = 0;
			break;
		case '1':
			digits[1] = digits[0];
			digits[0] = 1;
			break;
		case '2':
			digits[1] = digits[0];
			digits[0] = 2;
			break;
		case '3':
			digits[1] = digits[0];
			digits[0] = 3;
			break;
		case '4':
			digits[1] = digits[0];
			digits[0] = 4;
			break;
		case '5':
			digits[1] = digits[0];
			digits[0] = 5;
			break;
		case '6':
			digits[1] = digits[0];
			digits[0] = 6;
			break;
		case '7':
			digits[1] = digits[0];
			digits[0] = 7;
			break;
		case '8':
			digits[1] = digits[0];
			digits[0] = 8;
			break;
		case '9':
			digits[1] = digits[0];
			digits[0] = 9;
			break;
	}
	printf("digits: %u, %u, \n", digits[1], digits[0]);
	
}

void add_new_digit(uint8_t new_digit, uint8_t digits[2]){
	digits[1] = digits[0];
	digits[0] = new_digit;
}

float make_float_from_last_two_digits(uint8_t digits[2]){
	// TODO:
	return 0.f;
}
