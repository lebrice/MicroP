#ifndef _BLINKER_H
#define _BLINKER_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"



#define blinker_on_signal 0x0000



// Very useful macros for setting and resetting a given pin.
#define PIN(i) i##_Pin
#define PORT(i) i##_GPIO_Port
#define SET_PIN(i) HAL_GPIO_WritePin(PORT(i), PIN(i), GPIO_PIN_SET)
#define RESET_PIN(i) HAL_GPIO_WritePin(PORT(i), PIN(i), GPIO_PIN_RESET)
#define TOGGLE_PIN(i) HAL_GPIO_TogglePin(PORT(i), PIN(i))
#define READ_PIN(i) HAL_GPIO_ReadPin(PORT(i), PIN(i))


void start_blinking(void);
void stop_blinking(void);


void StartBlinkerTask(void const * argument);



#endif
