
#include <stdint.h>
// TODO: include all the functions and stuff for reading the keypad.

// state machine:
// INIT --(digit entered)--> one digit
// one_digit --(digit entered)--> Two digits
// two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
// two_digits --(pound sign)--> INIT

// TODO: FIX these values using the documentation.
#define KEYPAD_0 0x00
#define KEYPAD_1 0x01
#define KEYPAD_2 0x02
#define KEYPAD_3 0x03
#define KEYPAD_4 0x04
#define KEYPAD_5 0x05
#define KEYPAD_6 0x06
#define KEYPAD_7 0x07
#define KEYPAD_8 0x08
#define KEYPAD_9 0x09
#define KEYPAD_POUND 0xFF
#define KEYPAD_STAR 0xFE


extern float input_value;

