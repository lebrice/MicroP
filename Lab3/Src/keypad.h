
#include <stdint.h>


#define KEYPAD 0


/** PIN NAME	|	BREAD_BOARD LANE |	BOARD PHYSICAL PIN
* 	COL_0				6										PB13
* 	COL_1				7										PB14
* 	COL_2				8										PB15
* 	ROW_0				2										PB9 (Not close to the others)
* 	ROW_1				3										PB10
* 	ROW_2				4										PB11
* 	ROW_3				5										PB12*
*/

// TODO: FIX these values using the documentation.
static const uint8_t ROWS = 4;
static const uint8_t COLS = 3; 

static const char Keys[ROWS][COLS] = {
	{'1', '2', '3'},
	{'4', '5', '6'},
	{'7', '8', '9'},
	{'*', '0', '#'}
};


#define CHECK_FOR_DIGIT_PRESS_INTERVAL_MS 25

static const int SLEEP_PRESS_DURATION_MS = 3000;
static const int RESTART_PRESS_DURATION_MS = 1000;
static const int DEBOUNCE_INTERVAL_MS = 200;

extern float dac_target_value;

void keypad_update(char new_keypad_value);
