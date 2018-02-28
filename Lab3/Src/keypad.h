
#include <stdint.h>


#define KEYPAD 0

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __stdio_h
#include <stdio.h>
#endif 

#ifndef __MAIN_H__
#include "main.h"
#endif

#ifndef __STM32F4xx_HAL_H
#include "stm32f4xx_hal.h"
#endif 

#ifndef FMS
#include "fsm.h"
#endif

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
static void pwm_duty_cycle(uint16_t percentage);
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

void check_for_digit_press(void);
void keypad_update(char new_keypad_value);


extern uint8_t digits[3];
