/**
This file defines the functions and constants used in the 7-segment display.
*/
#include <stdint.h>

#define SEG_A 0x01
#define SEG_B 0x02
#define SEG_C 0x04
#define SEG_D 0x08
#define SEG_E 0x10
#define SEG_F 0x20
#define SEG_G 0x40

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


// Here we define constants for the Display Mode used to toggle the display.
#define DISPLAY_RMS 0
#define DISPLAY_MIN 1
#define DISPLAY_MAX 2

#define DISPLAY_REFRESH_INTERVAL_MS 5

// Very useful macros for setting and resetting a given pin.
#define PIN(i) i##_Pin
#define SET_PIN(i) HAL_GPIO_WritePin(i##_GPIO_Port, PIN(i), GPIO_PIN_SET)
#define RESET_PIN(i) HAL_GPIO_WritePin(i##_GPIO_Port, PIN(i), GPIO_PIN_RESET)


/** Represents which value we wish to display.
When 0, the RMS is displayed.
When 1, the MIN is displayed.
Whwn 2, the MAX is displayed.
*/
extern short display_mode;

extern float displayed_value;

void get_segments_for_float(float value, uint8_t segments[3]);
