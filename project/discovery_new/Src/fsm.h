
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




typedef enum {
	IDLE,
	TAP_RECENT,
	SINGLETAP,
	DOUBLETAP,
	WAITING_REPLY,
} STATE;

