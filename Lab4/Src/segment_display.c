#include "segment_display.h"

#ifndef __math_h
#include "math.h"
#endif 

bool display_on;

uint8_t display_mode = DISPLAY_RMS;
float displayed_value = 0.f;

// Easy way of converting from a digit to a segment.
const int digit_to_segments[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};



void stop_display(){
	display_on = false;
}

void start_display(){
	display_on = true;
}


void split_three_digits(float value, int digits[3]){
	int int_value = round(value * 100);
	digits[2] = round(int_value / 100);
	int_value -= digits[2] * 100;
	digits[1] = round(int_value / 10);
	int_value -= digits[1] * 10;
	digits[0] = round(int_value % 10);
//	printf("%f gets translated to %u, %u, %u\n", value, digits[2], digits[1], digits[0]);
}

void get_segments_for_float(float value, uint8_t segments[3]){
	int digits[3];
	split_three_digits(value, digits);
//	segments[2] = digit_to_segments[digits[2]];
//	segments[1] = digit_to_segments[digits[1]];
//	segments[0] = digit_to_segments[digits[0]];
	
	
	for(int i = 0; i < 3; i++){
		segments[i] = digit_to_segments[digits[i]];
//		printf("digit: %u, Segments: %02x\n", digits[i], segments[i]);
	}
}

