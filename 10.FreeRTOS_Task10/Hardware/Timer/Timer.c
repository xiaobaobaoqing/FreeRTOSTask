#include "Timer.h"



TIM_HandleTypeDef TIM2_Base;
TIM_HandleTypeDef TIM3_Base;

volatile uint8_t TIM2_flag=0;
volatile uint8_t TIM3_flag=0;

void Timer2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();


    TIM2_Base.Instance = TIM2;
    TIM2_Base.Init.Prescaler = 10000U - 1;
    TIM2_Base.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM2_Base.Init.Period = 10000U - 1;
    TIM2_Base.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM2_Base);


    HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    HAL_TIM_Base_Start_IT(&TIM2_Base);

}

void Timer3_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();


    TIM3_Base.Instance = TIM3;
    TIM3_Base.Init.Prescaler = 10000U - 1;
    TIM3_Base.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM3_Base.Init.Period = 10000U - 1;
    TIM3_Base.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM3_Base);


    HAL_NVIC_SetPriority(TIM3_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    HAL_TIM_Base_Start_IT(&TIM3_Base);

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        TIM2_flag=1;
    }
    if (htim->Instance == TIM3)
    {
        TIM3_flag=1;
    }
}