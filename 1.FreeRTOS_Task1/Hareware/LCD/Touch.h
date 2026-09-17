#ifndef __TOUCH_H__
#define __TOUCH_H__


#include "stm32f4xx.h"
#include "Touch_I2C.h"
#include "Delay.h"
#include "LCD.h"
#include "LCD_Define.h"


#define CST816D_RST             GPIO_Pin_15
#define CST816D_SCL             GPIO_Pin_6
#define CST816D_SDA             GPIO_Pin_4
#define CST816D_RST_GPIO_Port   GPIOA
#define CST816D_IIC_GPIO_Port   GPIOB

/* 触摸屏寄存器 */

 
#define CST816D_Address	                0x2A //触摸屏IIC写地址
#define CST816D_GestureID 			    0x01 //触摸屏中与手势识别
#define CST816D_FingerNum 			    0x02 //记录触摸手指数量
#define CST816D_XposH 					0x03 //X 坐标高位相关的寄存器地址
#define CST816D_XposL 					0x04 //X 坐标低位部分的寄存器地址
#define CST816D_YposH 					0x05 //Y 坐标高位相关的寄存器地址
#define CST816D_YposL 					0x06 //Y 坐标低位部分的寄存器地址
#define CST816D_ChipID 					0xA7 //访问触摸屏芯片的唯一标识符寄存器的地址
#define CST816D_SleepMode				0xE5 //控制触摸屏进入或退出睡眠模式的寄存器地址
#define CST816D_MotionMask 			    0xEC //对某些运动相关操作的屏蔽或启用设置
#define CST816D_IrqPluseWidth 	        0xED //中断低脉冲宽度相关的寄存器地址
#define CST816D_NorScanPer 		    	0xEE //触摸屏正常扫描周期相关的寄存器地址
#define CST816D_MotionSlAngle 	        0xEF //涉及到运动滑动角度相关
#define CST816D_LpAutoWakeTime      	0xF4 //长按自动唤醒时间
#define CST816D_LpScanTH 				0xF5 //触摸屏的长按扫描阈值
#define CST816D_LpScanWin 		    	0xF6 //触摸屏的长按扫描窗口相关
#define CST816D_LpScanFreq 		    	0xF7 //触摸屏的长按扫描频率
#define CST816D_LpScanIdac 		    	0xF8 //长按扫描电流
#define CST816D_AutoSleepTime       	0xF9 //触摸屏自动进入睡眠模式的时间相关
#define CST816D_IrqCtl 					0xFA //触摸屏的中断控制相关
#define CST816D_AutoReset 			    0xFB //触摸屏自动复位相关
#define CST816D_LongPressTime 	        0xFC //触摸屏的长按时间
#define CST816D_IOCtl 					0xFD //触摸屏的输入输出控制


typedef struct
{

    uint16_t Xpos;     // X 坐标
    uint16_t Ypos;     // Y 坐标
    uint8_t  Finger; // 触摸点数 0/1

}Touch_CoordInfoDef; //坐标控制器

typedef enum
{
    CST816D_GestureNone         = 0x00, //无手势
    CST816D_GestureMoveDown     = 0x01, //下划手势
    CST816D_GestureMoveUp       = 0x02, //上划手势
    CST816D_GestureMoveLeft     = 0x03, //左划手势
    CST816D_GestureMoveRight    = 0x04, //右划手势
    CST816D_GestureClick        = 0x05, //点击手势
    CST816D_GestureDoubleClick  = 0x0B, //双击手势
    CST816D_GestureLongPress    = 0x0C, //长按手势
} CST816D_GestureDef; //手势ID识别选项类型

typedef enum
{
   CST816D_MotionMaskDisable   = 0x00, //禁止所有连续动作
   CST816D_MotionMaskEnConLR   = 0x01, //启用左右连续动作
   CST816D_MotionMaskEnConUD   = 0x02, //启用上下连续动作
   CST816D_MotionMaskEnClick   = 0x04, //启用连续点击动作
   CST816D_MotionMaskEnAll     = 0x07, //启用所有连续动作

} CST816D_MotionMaskDef; //连续动作配置选项

typedef enum
{   
    CST816D_IrqCtlOnceWLP      = 0x00, //长按手势只发出一个低脉冲信号。
    CST816D_IrqCtlENMotion     = 0x10, //检测到手势时，发出低脉冲。
    CST816D_IrqCtlENChange     = 0x20, //检测到触摸状态变化时，发出低脉冲。
    CST816D_IrqCtlENTouch      = 0x40, //检测到触摸时，周期性发出低脉冲。
    CST816D_IrqCtlENTest       = 0x80, //中断引脚测试，使能后自动周期性发出低脉冲

}CTST816D_IrqCtlDef; //中断控制选项

extern Touch_CoordInfoDef Touch_Data;

void Touch_Init(void);
uint8_t Touch_GetID(void);
void Touch_GetCoord(Touch_CoordInfoDef *Touch_CoordInfo);
uint8_t Touch_Get_FingerNum(void);

void Touch_SCAN(void);        // 触摸扫描函数


void Touch_ConfigMotionMask(uint8_t Mask);
void Touch_ConfigSleepMode(void);

void Touch_WakeUpMode(void);
void Touch_ConfigMotionSlAngle(uint8_t Angle);

void Touch_ConfigNorScanPer(uint8_t Period);

void Touch_ConfigIrqPluseWidth(uint8_t Width);
void Touch_ConfigLpScanTh(uint8_t Th);

void Touch_Reset(void);


#endif 


