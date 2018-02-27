#define FSM
#include "fsm.h"

static STATE current_state = SLEEP;

static float dac_target_value;


// externally-defined functions for startign and stopping the ADC.
extern void start_adc(void);
extern void stop_adc(void);

void sleep(void);



void sleep(){
	current_state = SLEEP;
	stop_adc();
}

void wake_up(){
	current_state = INPUT_TARGET;
}

void start_matching(){
	current_state = MATCH_VOLTAGE;
	start_adc();
}
