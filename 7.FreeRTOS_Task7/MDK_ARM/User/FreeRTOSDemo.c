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
#define Priority_Task2 3
TaskHandle_t Task2Handle = NULL;
void Task2(void *pvParameters);

List_t TestList;
ListItem_t TestItem1;
ListItem_t TestItem2;
ListItem_t TestItem3;


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

    vListInitialise(&TestList);
    vListInitialiseItem(&TestItem1);
    vListInitialiseItem(&TestItem2);
    vListInitialiseItem(&TestItem3);
    TestItem1.xItemValue = 40;
    TestItem2.xItemValue = 60;
    TestItem3.xItemValue = 50;

    vTaskDelay(10000);
    Serial_Printf("TestList\t\t0x%p\t\r\n",&TestList);
    Serial_Printf("TestList->pxIndex\t0x%p\t\r\n",TestList.pxIndex);
    Serial_Printf("TestList->xListEnd\t0x%p\t\r\n",&(TestList.xListEnd));
    Serial_Printf("TestItem1\t\t0x%p\t\r\n",&TestItem1);
    Serial_Printf("TestItem2\t\t0x%p\t\r\n",&TestItem2);
    Serial_Printf("TestItem3\t\t0x%p\t\r\n",&TestItem3);

    Serial_Printf("插入列表项一\r\n",&TestItem3);
    vListInsert(&TestList,&TestItem1);
    Serial_Printf("TestList->xListEnd->pxNext\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxNext\t0x%p\t\r\n",TestItem1.pxNext);
    Serial_Printf("TestList->xListEnd->pxPrevious\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxPrevious\t0x%p\t\r\n",TestItem1.pxPrevious);

    Serial_Printf("插入列表项二\r\n",&TestItem2);
    vListInsert(&TestList,&TestItem2);
    Serial_Printf("TestList->xListEnd->pxNext\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxNext\t0x%p\t\r\n",TestItem1.pxNext);
    Serial_Printf("ListItem2->pxNext\t0x%p\t\r\n",TestItem2.pxNext);
    Serial_Printf("TestList->xListEnd->pxPrevious\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxPrevious\t0x%p\t\r\n",TestItem1.pxPrevious);
    Serial_Printf("ListItem2->pxPrevious\t0x%p\t\r\n",TestItem2.pxPrevious);

    Serial_Printf("插入列表项三\r\n",&TestItem3);
    vListInsert(&TestList,&TestItem3);
    Serial_Printf("TestList->xListEnd->pxNext\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxNext\t0x%p\t\r\n",TestItem1.pxNext);
    Serial_Printf("ListItem2->pxNext\t0x%p\t\r\n",TestItem2.pxNext);
    Serial_Printf("ListItem3->pxNext\t0x%p\t\r\n",TestItem3.pxNext);
    Serial_Printf("TestList->xListEnd->pxPrevious\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxPrevious\t0x%p\t\r\n",TestItem1.pxPrevious);
    Serial_Printf("ListItem2->pxPrevious\t0x%p\t\r\n",TestItem2.pxPrevious);
    Serial_Printf("ListItem3->pxPrevious\t0x%p\t\r\n",TestItem3.pxPrevious);

    Serial_Printf("删除列表项二\r\n",&TestItem2);
    vListRemove(&TestItem2);
    Serial_Printf("TestList->xListEnd->pxNext\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxNext\t0x%p\t\r\n",TestItem1.pxNext);
    Serial_Printf("ListItem3->pxNext\t0x%p\t\r\n",TestItem3.pxNext);
    Serial_Printf("TestList->xListEnd->pxPrevious\t0x%p\t\r\n",TestList.xListEnd.pxNext);
    Serial_Printf("ListItem1->pxPrevious\t0x%p\t\r\n",TestItem1.pxPrevious);
    Serial_Printf("ListItem3->pxPrevious\t0x%p\t\r\n",TestItem3.pxPrevious);







    while(1)
    {
        vTaskDelay(1000);


    }
}

