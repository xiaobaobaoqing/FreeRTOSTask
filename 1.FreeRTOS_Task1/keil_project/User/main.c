#include "stm32f4xx.h"                  // Device header
#include "LCD.h"
#include "FreeRTOS.h"
#include "task.h"


int main()
{
    LCD_Init();
    LCD_Clear(RED);

    while(1)
    {
        
    }
}

