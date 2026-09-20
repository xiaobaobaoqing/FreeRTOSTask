#include "stm32f4xx_hal.h"
#include "FreeRTOSConfig.h"

#define OS_SUPPORT 	1
#define SYS_CLK    100

static uint32_t fac_us=0;                          //us延时倍乘数

#if OS_SUPPORT 						    								   
static uint32_t fac_ms=0;                          //ms延时倍乘数
#endif
/**
 * @brief  延时模块初始化，只需调用一次，main中HAL初始化之后调用
 * @retval 无
 */
void Delay_Init(void)
{
	uint32_t reload;
    const uint8_t SYSCLK = 100;                          // F411CEU6 固定100M
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
    fac_us=SYSCLK;                                  //不论是否使用OS,fac_us都需要使用
    reload=SYSCLK;                                  //每秒钟的计数次数 单位为M
    reload*=1000000/configTICK_RATE_HZ;             //根据configTICK_RATE_HZ设定溢出时间
                                                    //reload为24位寄存器,最大值:16777216
    fac_ms=1000/configTICK_RATE_HZ;                 //代表OS可以延时的最少单位
    SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;        //开启SYSTICK中断
    SysTick->LOAD=reload;                           //每1/configTICK_RATE_HZ断一次
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;         //开启SYSTICK
}



	 
void Delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				    	 
	ticks=nus*SYS_CLK; 						
	told=SysTick->VAL;        				
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	}
}

void Delay_ms(uint16_t nms)
{
	uint32_t i;
	for(i=0;i<nms;i++) Delay_us(1000);
}
