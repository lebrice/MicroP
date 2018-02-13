/**
This file defines the functions and constants used in the 7-segment display.
*/


#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000

#define ZERO 	(SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F)
#define ONE 	(SEG_B|SEG_C)
#define TWO		(SEG_A|SEG_B|SEG_G|SEG_E|SEG_D)
#define THREE (SEG_A|SEG_B|SEG_C|SEG_D|SEG_G)
#define FOUR 	(SEG_B|SEG_C|SEG_G|SEG_F)
#define FIVE 	(SEG_A|SEG_F|SEG_G|SEG_C|SEG_D)
#define SIX		(SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G)
#define SEVEN	(SEG_A|SEG_B|SEG_C)
#define EIGHT (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G)
#define NINE 	(SEG_A|SEG_B|SEG_C|SEG_F|SEG_G)

