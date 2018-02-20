
#include <stdint.h>


#define KEYPAD 0

// TODO: include all the functions and stuff for reading the keypad.

// state machine:
// INIT --(digit entered)--> one digit
// one_digit --(digit entered)--> Two digits
// two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
// two_digits --(pound sign)--> INIT

// TODO: FIX these values using the documentation.
static const uint8_t ROWS = 4;
static const uint8_t COLS = 3; 

static const char Keys[ROWS][COLS] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};


#define CHECK_FOR_DIGIT_PRESS_INTERVAL_MS 200


extern float input_value;

void new_keypad_value(char new_keypad_value);
