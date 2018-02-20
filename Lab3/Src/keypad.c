// keypad.c
#include "keypad.h"

float input_value = 0.0f;


void new_keypad_value(uint8_t new_keypad_value){
	static uint8_t digits[2];
	uint8_t new_value
	switch(new_keypad_value){
		case KEYPAD_POUND:
			input_value = make_float_from_last_two_digits();
			break;
		case KEYPAD_0:
			
	}
	
	
}

float make_float_from_last_two_digits(){
	// TODO:
	return 0.f;
}
