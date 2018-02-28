// keypad.c
#include "keypad.h"

float make_float_from_last_three_digits(uint8_t digits[3]);

bool is_valid_target_value(float target_value);


// the three displayed digits.
uint8_t digits[3];


/** @brief Called to check wether one key on the keypad is pressed down.
*
* IDEA: Set one column high, and check if any rows are high. If so, the button at the crossing is pressed.
*/
void check_for_digit_press(){
	static const uint32_t rows[] = { ROW_0_Pin, ROW_1_Pin, ROW_2_Pin, ROW_3_Pin };
	static const uint32_t columns[] = { COL_0_Pin, COL_1_Pin, COL_2_Pin };
	
	static bool key_pressed_in_rows[4];
	
	static uint8_t current_row;
	static uint8_t column;
	
	bool press_detected = false;
		
	// The character that is being pushed in the keypad.
	char new_char = NULL;

	// Reset all columns, and set the current column to HIGH.
	for(int i=0; i<ROWS; i++){
		HAL_GPIO_WritePin(GPIOB, rows[i], (i == current_row) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
	
	
	// Read each row. If a row is high, we found the digit.
	for(column = 0; column < COLS; column++){
		// TODO: figure out why the ReadPin returns PIN_RESET when the pin is HIGH, and PIN_SET when pin is LOW.
		if(HAL_GPIO_ReadPin(GPIOB, columns[column]) == GPIO_PIN_SET){
			// printf("KEY (%u, %u) is ON.\n", current_row, column);
			new_char = Keys[current_row][column];
			press_detected = true;
			break;
		}else{
			// printf("KEY (%u, %u) is OFF.\n", current_row, column);
		}
	}
	if(press_detected){
		key_pressed_in_rows[current_row] = true;
		keypad_update(new_char);
	}else{
		key_pressed_in_rows[current_row] = false;
		// If we have not detected a keypress in all 4 rows, signal that no key is pressed.
		if(!(key_pressed_in_rows[0]			
			|	key_pressed_in_rows[1]
			| key_pressed_in_rows[2]
			|	key_pressed_in_rows[3])){
			new_char = ' ';
			keypad_update(new_char);
		}
	}
	
	current_row++;
	current_row %= ROWS;
}



/** @brief Called whenever a new keypad value is received.
* IDEA: state machine:
* INIT --(digit entered)--> one digit
* one_digit --(digit entered)--> Two digits
* two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
* two_digits --(pound sign)--> INIT
*/
void keypad_update(char new_keypad_value){
	
	// number of consecutive updates required for a 'press' to be registered as such. (debouncing.)
	static const int min_updates_for_change = DEBOUNCE_INTERVAL_MS / (ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);
	// number of consecutive times a '*' needs to be received in order to restart.
	static const int min_updates_for_restart = RESTART_PRESS_DURATION_MS / (ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);
	// number of consecutive times a '*' needs to be received in order to go to sleep.
	static const int min_updates_for_sleep = SLEEP_PRESS_DURATION_MS / (ROWS * CHECK_FOR_DIGIT_PRESS_INTERVAL_MS);
	
	// the displayed value.
	extern float displayed_value;
	
	// the current state of the FSM.
	extern STATE current_state;	
	
	
	
	
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
		
		if (current_state == SLEEP && new_keypad_value != ' '){
			// a button was pressed. If we were sleeping, wake up.
			wake_up();
		}
		
		
		switch(new_keypad_value){	
			
			case '#':
				// We use the last three digits to assign the input value.
				new_target = make_float_from_last_three_digits(digits);
				if (is_valid_target_value(new_target)){
						start_matching(new_target);
				}
				// Switch to the "view adc level" mode of operation.
				
				break;
				
				
			case '*':
				if (last_digit_updates_count >= min_updates_for_sleep){
					// TODO: sleep.
					sleep();
				}else if (last_digit_updates_count >= min_updates_for_restart){
					// restart this state.
					digits[0] = 0;
					digits[1] = 0;
					digits[2] = 0;
					restart();
					// TODO: not sure if we should set the 'current_state' value.					
				}else if (last_digit_updates_count == min_updates_for_change){
					// We want to remove the last digit.
					digits[0] = digits[1];
					digits[1] = digits[2];
					digits[2] = 0;
				}	  
				break;
			
			
			case ' ':
				break;
			
			
			default:
				// A digit was pressed ('0' to '9')
				if (last_digit_updates_count == min_updates_for_change){
					digits[2] = digits[1];
					digits[1] = digits[0];
					digits[0] = (int)(new_keypad_value - '0');
				}
		}
	}
	// Display the input value if we're in the "input target" state.
	if(current_state == INPUT_TARGET){
		displayed_value = make_float_from_last_three_digits(digits);
	}
}


bool is_valid_target_value(float target_value){
	return (target_value > 0.f) && (target_value < 3.0f);
}



float make_float_from_last_three_digits(uint8_t digits[3]){
	// TODO:
	float value = 1.0f * digits[2] + 0.1f * digits[1] + 0.01f * digits[0];
	return value;
}
