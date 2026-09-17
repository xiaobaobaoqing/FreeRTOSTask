#include "stm32f4xx_hal.h"
#include "LCD.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

void FreeRTOS_Demo(void);


#define Stack_StartTask 128
#define Priority_StartTask 1
TaskHandle_t StartTaskHandle = NULL;
StackType_t  StartTaskStack[Stack_StartTask];
StaticTask_t StartTaskTCB;
void StartTask(void *pvParameters);

#define Stack_Task1 128
#define Priority_Task1 2
TaskHandle_t Task1Handle = NULL;
StackType_t  Task1Stack[Stack_Task1];
StaticTask_t Task1TCB;
void Task1(void *pvParameters);

#define Stack_Task2 128
#define Priority_Task2 3
TaskHandle_t Task2Handle = NULL;
StackType_t  Task2Stack[Stack_Task2];
StaticTask_t Task2TCB;
void Task2(void *pvParameters);



StaticTask_t idleTaskTCB;
StackType_t idleTaskStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, 
                                    StackType_t ** ppxIdleTaskStackBuffer, 
                                    configSTACK_DEPTH_TYPE * puxIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &idleTaskTCB;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

StaticTask_t TimerTaskTCB;
StackType_t TimerTaskStack[configMINIMAL_STACK_SIZE];
void vApplicationGetTimerTaskMemory(     StaticTask_t ** ppxTimerTaskTCBBuffer,
                                         StackType_t ** ppxTimerTaskStackBuffer,
                                         configSTACK_DEPTH_TYPE * puxTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &TimerTaskTCB;
    *ppxTimerTaskStackBuffer = TimerTaskStack;
    *puxTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}


int main(void)
{
    HAL_Init();

    SystemClock_Config(); 
    LCD_Init();

    FreeRTOS_Demo();

    while(1)
    {

    }
}


void FreeRTOS_Demo(void)
{
    StartTaskHandle = xTaskCreateStatic(StartTask, "StartTask",Stack_StartTask,NULL,Priority_StartTask,StartTaskStack,&StartTaskTCB);
    vTaskStartScheduler();
}

void StartTask(void *pvParameters)
{
    taskENTER_CRITICAL();

    Task1Handle = xTaskCreateStatic(Task1, "Task1",Stack_Task1,NULL,Priority_Task1,Task1Stack,&Task1TCB);
    Task2Handle = xTaskCreateStatic(Task2, "Task2",Stack_Task2,NULL,Priority_Task2,Task2Stack,&Task2TCB);

    vTaskDelete(StartTaskHandle);
    taskEXIT_CRITICAL();

} 

void Task1(void *pvParameters)
{

    while(1)
    {

        LCD_LED(20,20,RED);
        vTaskDelay(1000);
        LCD_LED(20,20,BLACK);
        vTaskDelay(1000);
    }
}

void Task2(void *pvParameters)
{


    while(1)
    {

        LCD_LED(50,50,GREEN);
        vTaskDelay(100);
        LCD_LED(50,50,BLACK);
        vTaskDelay(100);
    }
}































void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* HSI做系统PLL主时钟；同时开启外部32.768K LSE晶振（给RTC用） */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI; // PLL来源依旧HSI(16M)
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 100;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
      Error_Handler();
    }
    SystemCoreClockUpdate();
}




void Error_Handler(void)
{
  while (1);
}