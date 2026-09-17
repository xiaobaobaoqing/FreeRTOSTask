#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

static uint32_t g_fac_us = 0; 
static uint16_t i = 0; 

void Delay_Init(void)
{	

	u32 reload;

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	reload = SystemCoreClock;

	reload = reload / configTICK_RATE_HZ;

	SysTick->LOAD = reload;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	g_fac_us = SystemCoreClock / 1000000;
	
}

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{    
	
	
	if(i==0)
	{
		i++;
		Delay_Init();
	}
	uint32_t ticks;
	uint32_t reload;
	uint32_t told,tnow,tcnt=0;

	reload = SysTick->LOAD;
	ticks = xus *g_fac_us;
	told = SysTick->VAL;
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			
			if(tnow < told)
			{
				tcnt += told - tnow;
			}
			else
			{
				tcnt += reload - tnow + told;
			}
			told = tnow;
			if(tcnt >= ticks)
			{
				break;
			}
		}

	}
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 
