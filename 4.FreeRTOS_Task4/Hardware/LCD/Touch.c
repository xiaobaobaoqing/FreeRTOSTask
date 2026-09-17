#include "Touch.h"
#include "stm32f4xx.h"                  // Device header
#include "Delay.h"

Touch_CoordInfoDef Touch_Data;



void Touch_ConfigAutoSleepTime(uint8_t Time)
{
    CST816D_I2C_WriteReg(CST816D_AutoSleepTime, Time);
}


/**
  * 函    数：复位函数
  * 参    数：无
  * 返 回 值：无
**/
void Touch_Reset(void)
{
    GPIO_WriteBit(CST816D_RST_GPIO_Port, CST816D_RST, Bit_RESET);
    Delay_ms(10);
    GPIO_WriteBit(CST816D_RST_GPIO_Port, CST816D_RST, Bit_SET);
    Delay_ms(10);

}
/**
  * 函    数：初始化函数
  * 参    数：无
  * 返 回 值：无
**/
void Touch_Init(void)
{
    CST816D_I2C_Init();// 初始化CST816D触摸屏// 这里可以添加I2C初始化代码和CST816D的配置代码
}

/**
  * 函    数：读取坐标信息
  * 参    数：CST816D_CoordInfoDef *CST816D_CoordInfo：坐标信息结构体指针
  * 返 回 值：无
**/

void Touch_GetCoord(Touch_CoordInfoDef *Touch_CoordInfo)
{
    uint8_t Data[6];
    
    // 从 0x01 寄存器开始读取 6 个字节
    // Data[0]: 0x01 (保留/忽略)
    // Data[1]: 0x02 (手指数量) -> 实际上有的库从0x03开始读，取决于你的 ReadArray 起点
    // 这里假设你的 ReadArray(0x01, Data, 6) 对应寄存器 0x01-0x06
    CST816D_I2C_ReadArray(0x01, Data, 6); 

    /* 
       根据 CST816D 寄存器手册：
       寄存器 0x03: Xpos High [3:0] (高4位是 Event Flag，必须清零)
       寄存器 0x04: Xpos Low  [7:0]
       寄存器 0x05: Ypos High [3:0] (高4位是 Touch ID，必须清零)
       寄存器 0x06: Ypos Low  [7:0]
    */

    uint16_t rawX = ((uint16_t)(Data[2] & 0x0F) << 8) | Data[3];
    uint16_t rawY = ((uint16_t)(Data[4] & 0x0F) << 8) | Data[5];

    // --- Y 轴校准逻辑 ---
    
    // 1. 假设你测得 LCD 最顶端对应的原始值是 Y_TOP_RAW
    // 假设你测得 LCD 最底端对应的原始值是 Y_BOTTOM_RAW
    // 根据你之前的数据，我们尝试调整这两个值：
    const int16_t Y_TOP_RAW = 0;      // 顶端原始值
    const int16_t Y_BOTTOM_RAW = 290; // 底端原始值

    int32_t calibratedY;

    // 如果上方有一大部分是 0，尝试把整体坐标向下推
    // 修正公式：(rawY - Y_TOP_RAW) * 320 / (Y_BOTTOM_RAW - Y_TOP_RAW)
    calibratedY = (int32_t)(rawY - Y_TOP_RAW) * 320 / (Y_BOTTOM_RAW - Y_TOP_RAW);

    // 限制边界
    if (calibratedY < 0) calibratedY = 0;
    if (calibratedY > 319) calibratedY = 319;

    Touch_CoordInfo->Xpos = rawX;
    Touch_CoordInfo->Ypos = (uint16_t)calibratedY;
}

/**
  * 函    数：读取触摸手指数量
  * 参    数：无
  * 返 回 值：触摸手指数量
*/
uint8_t Touch_Get_FingerNum(void)
{
    return CST816D_I2C_ReadReg(CST816D_FingerNum); // 从寄存器读取触摸手指数量
}


/**
  * 函    数：读取芯片ID
  * 参    数：无
  * 返 回 值：芯片ID
  */

uint8_t Touch_GetID(void)
{
    // 这里可以添加对id的处理代码，例如打印或存储
    return  CST816D_I2C_ReadReg(CST816D_ChipID);

}

/*  
  * 函    数：配置触摸屏的工作模式
  * 参    数：Mode：工作模式
  * 返 回 值：无
*/
void Touch_ConfigMotionMask(uint8_t Mask)
{
    CST816D_I2C_WriteReg(CST816D_MotionMask, Mask);
}
/*
    * 函    数：配置触摸屏的工作模式
    * 参    数：Mode：工作模式
    * 返 回 值：无
*/
void Touch_ConfigSleepMode(void)
{
    CST816D_I2C_WriteReg(CST816D_SleepMode, 0x03); // 进入睡眠模式
}

void Touch_WakeUpMode(void)
{
    Touch_Reset(); // 复位CST816D
}

/**
 * 函数功能：配置触摸屏的滑动角度
 * 输入参数：Angle：滑动角度值
 * 返回值：无
 */

void Touch_ConfigMotionSlAngle(uint8_t Angle)
{
    CST816D_I2C_WriteReg(CST816D_MotionSlAngle, Angle);
}

/*  
  * 函    数：正常模式下，配置触摸屏的扫描周期
  * 参    数：period：扫描周期值
  * 返 回 值：无
*/
void Touch_ConfigNorScanPer(uint8_t Period)
{
    if(Period>=30)
        Period=30;
    CST816D_I2C_WriteReg(CST816D_NorScanPer, Period);
}

/**
  * 函    数：中断脉冲宽度
  * 参    数：Width：脉冲宽度值
  * 返 回 值：无
  */
void Touch_ConfigIrqPluseWidth(uint8_t Width)
{
    if (Width>=200)
    {
        Width=200;
    }
    
    CST816D_I2C_WriteReg(CST816D_IrqPluseWidth, Width);
}
/**
  * 函    数：配置低功耗扫描的灵敏度
  * 参    数：Th：灵敏度值 Th越小识别灵敏
  * 返 回 值：无
  */
void Touch_ConfigLpScanTh(uint8_t Th)
{
    CST816D_I2C_WriteReg(CST816D_LpScanTH, Th);
}




void Touch_SCAN(void)
{
     Touch_GetCoord(&Touch_Data);
    Touch_Data.Finger = Touch_Get_FingerNum();
}










