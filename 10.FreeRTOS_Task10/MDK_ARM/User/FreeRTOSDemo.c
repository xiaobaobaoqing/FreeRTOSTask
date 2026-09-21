#include "FreeRTOSDemo.h"



#define Stack_StartTask 128
#define Priority_StartTask 1
TaskHandle_t StartTaskHandle = NULL;
void StartTask(void *pvParameters);

#define Stack_Task1 128
#define Priority_Task1 2
TaskHandle_t Task1Handle = NULL;
void Task1(void *pvParameters);


#define Stack_Task2 128
#define Priority_Task2 10
TaskHandle_t Task2Handle = NULL;
void Task2(void *pvParameters);



void FreeRTOS_Demo(void)
{
    xTaskCreate(StartTask,"StartTask",Stack_StartTask,NULL,Priority_StartTask,&StartTaskHandle);
    vTaskStartScheduler();
}

void StartTask(void *pvParameters)
{
    taskENTER_CRITICAL();

    xTaskCreate(Task1,"Task1",Stack_Task1,NULL,Priority_Task1,&Task1Handle);
    xTaskCreate(Task2,"Task2",Stack_Task2,NULL,Priority_Task2,&Task2Handle);

    vTaskDelete (NULL);
    taskEXIT_CRITICAL();

} 

void Task1(void *pvParameters)
{

    while(1)
    {
        LCD_LED(20,20,RED);
        vTaskDelay(500);
        LCD_LED(20,20,BLACK);
        vTaskDelay(500);

    }
}

void Task2(void *pvParameters)
{



    vTaskPrioritySet(Task1Handle,5);

    UBaseType_t num = uxTaskPriorityGet(Task1Handle);

    UBaseType_t num1= uxTaskGetNumberOfTasks();

    while(1)
    {
        LCD_Printf(10,60,LCD_8x16,"Task1 Priority: %d",num);
        LCD_Printf(10,78,LCD_8x16,"Task1 Number: %d",num1);

        vTaskDelay(1000);

    }
}

