#include "stm32f4xx_hal.h"


#define OS_SUPPORT 	0
#define SYS_CLK    100


/**
 * @brief  延时模块初始化，只需调用一次，main中HAL初始化之后调用
 * @retval 无
 */
void Delay_Init(void)
{
   	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	/* Configure the SysTick to have interrupt in 1ms time basis*/
	HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq));
}


#if OS_SUPPORT 						    								   
void Delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				   	 
	ticks=nus*SYS_CLK; 						 
	Delay_osschedlock();					
	told=SysTick->VAL;        				
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;//这里注意一下SYSTICK是一个递减的计数器就可以了.	
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			
		}  
	};
	Delay_osschedunlock();															    
}  

void Delay_ms(uint16_t nms)
{	
	if(Delay_osrunning&&Delay_osintnesting==0)    
	{		 
		if(nms>=fac_ms)						
		{ 
   			Delay_ostimedly(nms/fac_ms);	
		}
		nms%=fac_ms;						 
	}
	Delay_us((uint32_t)(nms*1000));			
}
#else  
	 
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
#endif
