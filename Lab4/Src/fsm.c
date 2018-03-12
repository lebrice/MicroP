#include "fsm.h"

STATE current_state = SLEEP;

float target_voltage;
uint8_t display_mode;

// externally-defined functions for startign and stopping the ADC.
extern void start_adc(void);
extern void stop_adc(void);


// externally-defined function for stopping the display
extern void start_display(void);
extern void stop_display(void);

extern void pwm_duty_cycle(uint8_t);
void sleep(void);


void restart(){
	// the digits of the display.
	extern uint8_t digits[3];
	// TODO: not sure if we're supposed to do anything here.
	if(current_state == MATCH_VOLTAGE){
		// Reset the pwm duty cycle to 0%.
		pwm_duty_cycle(0);
		stop_adc();
	}
	current_state = INPUT_TARGET;
	digits[0] = 0;
	digits[1] = 0;
	digits[2] = 0;
}

void sleep(){
	if(current_state == MATCH_VOLTAGE){
		stop_adc();
		pwm_duty_cycle(0);
	}
	current_state = SLEEP;
	stop_display();
}

void wake_up(){
	current_state = INPUT_TARGET;
	start_display();
}

void start_matching(float voltage){
	current_state = MATCH_VOLTAGE;
	target_voltage = voltage;
	start_adc();
}
