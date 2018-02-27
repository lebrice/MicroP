#ifndef FSM
#include "fsm.h"
#endif

static STATE current_state = SLEEP;

static float dac_target_value;


// externally-defined functions for startign and stopping the ADC.
extern void start_adc(void);
extern void stop_adc(void);

void sleep(void);


void restart(){
	// TODO: not sure if we're supposed to do anything here.
}

void sleep(){
	current_state = SLEEP;
	stop_adc();
}

void wake_up(){
	current_state = INPUT_TARGET;
}

void start_matching(float target_voltage){
	current_state = MATCH_VOLTAGE;
	dac_target_value = target_voltage;
	start_adc();
}
