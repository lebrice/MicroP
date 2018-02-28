#define FSM


#ifndef __stdint_h
#include <stdint.h>
#endif


typedef enum {
	SLEEP,
	INPUT_TARGET,
	MATCH_VOLTAGE	
}STATE;


// Current state of the FSM.
extern STATE current_state;

extern float dac_target_value;



void sleep(void);
void restart(void);
void wake_up(void);
void start_matching(float target_voltage);
