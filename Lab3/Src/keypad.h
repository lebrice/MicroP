
#include <stdint.h>
// TODO: include all the functions and stuff for reading the keypad.

// state machine:
// INIT --(digit entered)--> one digit
// one_digit --(digit entered)--> Two digits
// two_digits --(digit entered)--> two_digits (bump the first digit, keep last two digits only)
// two_digits --(pound sign)--> INIT


#define KEYPAD_0 0x00
#define KEYPAD_1 0x00
#define KEYPAD_2 0x00
#define KEYPAD_3 0x00
#define KEYPAD_4 0x00
#define KEYPAD_5 0x00
#define KEYPAD_6 0x00
#define KEYPAD_7 0x00
#define KEYPAD_8 0x00
#define KEYPAD_9 0x00
#define KEYPAD_POUND 0x00


extern static float input_value;

