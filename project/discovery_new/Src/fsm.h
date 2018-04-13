
#ifndef __adc_thread_h
#define __adc_thread_h
#endif

#ifndef __stdint_h
#include <stdint.h>
#endif

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __CMSIS_OS_H
#include "cmsis_os.h"
#endif

#ifndef __stm32f4xx_hal_h
#include "stm32f4xx_hal.h"
#endif

#ifndef __math_h
#include "math.h"
#endif

#define TAP_FILTER_ALPHA 0.9
#define ACC_FILTER_ALPHA 0.5
#define TAP_THRESHOLD 100.0

#define PRE_RECORDING_DELAY_MS 1000

typedef enum {
	IDLE,
	TAP_RECENT,
	SINGLETAP,
	DOUBLETAP,
	WAITING_REPLY,
} STATE;

typedef struct {
	float rms;
	float max_value;
	float min_value;
	int max_index;
	int min_index;	
} asm_output;

void asm_math(float *inputValues, int size, asm_output *results);

