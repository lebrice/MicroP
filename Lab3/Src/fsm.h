
#define FSM

#define ABS(x) ((x < 0)? -x : x)
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define BOUND(x, lower, upper) (MAX(MIN(x, upper), lower))


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
