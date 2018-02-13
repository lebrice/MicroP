#include "heads_up_display.h"
#include "math.h"


short display_mode = DISPLAY_RMS;
float displayed_value = 0.f;

// Easy way of converting from a digit to a segment.
const int digit_to_segments[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};


void split_three_digits(float value, int digits[3]){
	int int_value = (value * 100);
	digits[0] = int_value / 100;
	int_value -= digits[0] * 100;
	digits[1] = int_value / 10;
	int_value -= digits[1] * 10;
	digits[2] = int_value % 10;
}

void get_segments_for_float(float value, uint8_t segments[3]){
	int digits[3];
	split_three_digits(value, digits);
	for(int i = 0; i < 3; i++){
		segments[i] = digit_to_segments[digits[i]];
	}
}
