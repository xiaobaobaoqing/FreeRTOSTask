#ifndef Timer_H
#define Timer_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "LCD.h"

extern volatile uint8_t TIM2_flag;
extern volatile uint8_t TIM3_flag;

void Timer2_Init(void);
void Timer3_Init(void);

#endif

