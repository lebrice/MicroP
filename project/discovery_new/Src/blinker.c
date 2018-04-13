#include "blinker.h"

extern osThreadId blinkerTaskHandle;

volatile uint32_t blink_count = 0;

static volatile uint32_t blinker_off = 0;

#define blinker_on_signal 0x0000

static uint32_t BLINK_PERIOD_MS = 250;

void start_blinking(){
	osSignalSet(blinkerTaskHandle, blinker_on_signal);
}

void stop_blinking(){
	blinker_off = 1;
}



/** Simple Blinker thread: Blinks N times if 'blink_count' is set, otherwise blinks until it is turned off.
*
*/
void StartBlinkerTask(void const * argument){
	while(1){
		RESET_PIN(LED_RED);
		osSignalWait(blinker_on_signal, osWaitForever);
		do{
			// Blink until turned off.
			RESET_PIN(LED_RED);
			osDelay(BLINK_PERIOD_MS / 2);
			SET_PIN(LED_RED);
			osDelay(BLINK_PERIOD_MS / 2);
		}while(!blinker_off);
		blinker_off = 0;
	}
}
