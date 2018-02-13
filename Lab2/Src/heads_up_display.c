#include "heads_up_display.h"
#include "math.h"

short display_mode = DISPLAY_RMS;


void split_three_digits(float value, int digits[3]){
	int int_value;
	int_value = (int)(value * 100);
		
	// EX: 2.57 --> 257
	digits[0] = int_value / 100;
	int_value -= digits[0] * 100;
	digits[1] = int_value / 10;
	int_value -= digits[1] * 10;
	digits[2] = int_value % 10;
}
