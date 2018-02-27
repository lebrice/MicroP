#ifndef FSM
#include "fsm.h"
#endif

STATE current_state = SLEEP;



float target_voltage;


// externally-defined functions for startign and stopping the ADC.
extern void start_adc(void);
extern void stop_adc(void);


// externally-defined function for stopping the display
extern void start_display(void);
extern void stop_display(void);


void sleep(void);


void restart(){
	// TODO: not sure if we're supposed to do anything here.
}

void sleep(){
	current_state = SLEEP;
	stop_adc();
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
