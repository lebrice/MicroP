// keypad.c
#include "keypad.h"

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __stdio_h
#include <stdio.h>
#endif

// The value that is to be matched by the 
static float dac_target_value = 0.0f;

float make_float_from_last_three_digits(uint8_t digits[3]);

bool is_valid_target_value(float target_value);


/** @bried Called whenever a new keypad value is received.
* IDEA: state machine:
* INIT --(digit entered)--> one digit
* one_digit --(digit entered)--> Two digits
* two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
* two_digits --(pound sign)--> INIT
*/
void keypad_update(char new_keypad_value){
	// number of consecutive times a '*' needs to be received in order to restart.
	static const int min_updates_for_restart = RESTART_PRESS_DURATION_MS / (ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);
	// number of consecutive times a '*' needs to be received in order to go to sleep.
	static const int min_updates_for_sleep = SLEEP_PRESS_DURATION_MS / (ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);
	// number of consecutive updates required for a 'press' to be registered as such. (debouncing.)
	static const int min_updates_for_change = DEBOUNCE_INTERVAL_MS / (ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);
	
	
	// the three displayed digits.
	static uint8_t digits[3];
	
	// the displayed value.
	extern float displayed_value;
	
	
	
	// the last pressed digit.
	static char last_pressed_digit = ' ';
	// the number of times we have received this digit.
	static int last_digit_updates_count;
	// a temporary value used to check if the value makes sense.	
	float new_target;
	
	
	// If we encounter a new value, reset the count, if not, increment it.
	if(new_keypad_value != last_pressed_digit){
		printf("Last pressed digit was '%c', (%u milliseconds)\n", last_pressed_digit, last_digit_updates_count * ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);		
		
		last_pressed_digit = new_keypad_value;
		last_digit_updates_count = 1;
	}else{
		last_digit_updates_count++;
	}
	// if we have seen the same value enough times for it to be significant (debouncing)
	if(last_digit_updates_count >= min_updates_for_change){
		switch(new_keypad_value){
			case '#':
				// We use the last three digits to assign the input value.
				new_target = make_float_from_last_three_digits(digits);
				if (is_valid_target_value(new_target)){
					dac_target_value = new_target;
				}
				// TODO: Switch to the "view adc level" mode of operation.
				break;
			case '*':
				if (last_digit_updates_count >= min_updates_for_sleep){
					// TODO: sleep.
				}else if (last_digit_updates_count >= min_updates_for_restart){
					// TODO: restart.
				}else{
					// We want to remove the last digit.
					digits[0] = digits[1];
					digits[1] = digits[2];
					digits[2] = 0;
				}	  
				break;
			case ' ':
				// TODO: not sure what to do here.
				break;
			default:
				digits[2] = digits[1];
				digits[1] = digits[0];
				digits[0] = (int)(new_keypad_value - '0');
		}
	}
	
	printf("digits: %u, %u, %u, \n", digits[2], digits[1], digits[0]);
	//TODO: check that this makes sense with the FSM diagram. 
	displayed_value = make_float_from_last_three_digits(digits);
}

bool is_valid_target_value(float target_value){
	return (target_value > 0.f) && (target_value < 3.0f);
}



float make_float_from_last_three_digits(uint8_t digits[3]){
	// TODO:
	float value = 1.0f * digits[2] + 0.1f * digits[1] + 0.01f * digits[0];
	return value;
}
