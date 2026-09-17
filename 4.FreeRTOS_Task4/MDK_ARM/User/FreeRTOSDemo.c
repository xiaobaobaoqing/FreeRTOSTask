#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSDemo.h"
#include "Key.h"
#include "LCD.h"
#include "Bluetooth.h"



#define Stack_StartTask 128
#define Priority_StartTask 1
TaskHandle_t StartTaskHandle = NULL;
void StartTask(void *pvParameters);

#define Stack_Task1 128
#define Priority_Task1 2
TaskHandle_t Task1Handle = NULL;
void Task1(void *pvParameters);


#define Stack_Task2 128
#define Priority_Task2 3
TaskHandle_t Task2Handle = NULL;
void Task2(void *pvParameters);


#define Stack_Task3 128
#define Priority_Task3 4
TaskHandle_t Task3Handle = NULL;
void Task3(void *pvParameters);



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
    xTaskCreate(Task3,"Task3",Stack_Task3,NULL,Priority_Task3,&Task3Handle);

    vTaskDelete (NULL);
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


void Task3(void *pvParameters)
{
    uint8_t key_num = 0;
    Serial_Printf("Task3 started\r\n");  
    while(1)
    {
        key_num = Key();
        if(key_num == 1)
        {
            Serial_Printf("Key 1 Pressed\n");
            vTaskSuspend(Task1Handle);
        }
        if(key_num == 2)
        {
            Serial_Printf("Key 2 Pressed\n");
            vTaskResume(Task1Handle);
        }
        vTaskDelay(10);
    }
}
