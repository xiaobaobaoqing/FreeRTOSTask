#ifndef LCD_DEFINE_H_
#define LCD_DEFINE_H_


#include "stm32f4xx_hal.h"

// ======================== RGB565 颜色值定义 ========================
// 格式：16位色 R5 G6 B5 → 0xRRRRRGGGGGGBBBBB
#define WHITE       0xFFFF    // 白色
#define BLACK       0x0000    // 黑色
#define BLUE        0x001F    // 蓝色
#define RED         0xF800    // 红色
#define MAGENTA     0xF81F    // 品红/紫红
#define GREEN       0x07E0    // 绿色
#define CYAN        0x7FFF    // 青色/蓝绿色
#define YELLOW      0xFFE0    // 黄色
#define GRAY        0X8430    // 灰色
#define BRED        0XF81F    // 浅红色
#define GRED        0XFFE0    // 金黄色
#define GBLUE       0X07FF    // 天蓝色
#define BROWN       0XBC40    // 棕色
#define BRRED       0XFC07    // 浅棕色
#define DARKBLUE    0X01CF    // 深蓝色
#define LIGHTBLUE   0X7D7C    // 浅蓝色
#define GRAYBLUE    0X5458    // 灰蓝色

#define LIGHTGREEN  0X841F    // 浅绿色
#define LGRAY       0XC618    // 浅灰色
#define LGRAYBLUE   0XA651    // 浅灰蓝色
#define LBBLUE      0X2B12    // 淡蓝色

// ======================== LCD 控制器指令集 ========================
// 功能：控制屏幕的所有操作（初始化、显示、寻址、读写数据）
#define LCD_NOP     0x00    // 空指令，无操作
#define LCD_SWRESET 0x01    // 软件复位（重启屏幕）
#define LCD_RDDID   0x04    // 读取屏幕芯片ID
#define LCD_RDDST   0x09    // 读取屏幕状态

#define LCD_SLPIN   0x10    // 进入睡眠模式（低功耗）
#define LCD_SLPOUT  0x11    // 退出睡眠模式（唤醒屏幕）
#define LCD_PTLON   0x12    // 进入部分显示模式
#define LCD_NORON   0x13    // 进入正常显示模式

#define LCD_INVOFF  0x20    // 关闭颜色反转（正常显示）
#define LCD_INVON   0x21    // 开启颜色反转（反色显示）
#define LCD_DISPOFF 0x28    // 关闭显示（黑屏）
#define LCD_DISPON  0x29    // 打开显示（亮屏）
#define LCD_CASET   0x2A    // 设置列地址范围（X轴）
#define LCD_RASET   0x2B    // 设置行地址范围（Y轴）
#define LCD_RAMWR   0x2C    // 写入像素数据到屏幕
#define LCD_RAMRD   0x2E    // 从屏幕读取像素数据

#define LCD_PTLAR   0x30    // 设置部分显示区域
#define LCD_COLMOD  0x3A    // 设置颜色格式（RGB565/RGB666等）
#define LCD_MADCTL  0x36    // 设置屏幕显示方向（横屏/竖屏/镜像）


#define LCD_FRAMERATE   0xC6    // 设置屏幕刷新率
#define LCD_PORCH_SET   0xB2    // 控制行 / 帧前后的空白间隔，保证显示稳定


#define LCD_GATE_CONTROL  0xB7    //调节栅极电压
#define LCD_VCMOS_SET     0xBB    //调节VCOM电压
#define LCD_LCMCTRL       0xC0    //LCM控制
#define LCD_VDVVRHEN      0xC2    //VDV和VRH交流电压设置
#define LCD_VRHSET        0xC3    //VRH电压设置
#define LCD_VDVSET        0xC4    //VDV电压设置
#define LCD_PVGAM_CTRL    0xE0    //正极性伽马校正
#define LCD_NVGAM_CTRL    0xE1    //负极性伽马校正

#define LCD_POWER_CTR1    0xD0   //电源控制1


// ======================== 旋转方向参数定义 ========================
#define MADCTL_MY      0x80    // 上下翻转
#define MADCTL_MX      0x40    // 左右翻转
#define MADCTL_MV      0x20    // 行列互换（旋转）
#define MADCTL_ML      0x10
#define MADCTL_RGB     0x00    // RGB颜色顺序
#define MADCTL_BGR     0x08    // BGR颜色顺序
#define MADCTL_MH      0x04

// ======================== 240x240 屏幕偏移适配 ========================
// 默认开启 240x240 尺寸
#define USING_240X240

// 默认旋转角度 0
#define LCD_ROTATION 0

#ifdef USING_240X240
    #define LCD_WIDTH      240
    #define LCD_HEIGHT     280

    #if LCD_ROTATION == 0   // 0° 竖屏
        #define X_SHIFT      0
        #define Y_SHIFT      20
    #elif LCD_ROTATION == 1 // 90° 横屏
        #define X_SHIFT      20
        #define Y_SHIFT      0
    #elif LCD_ROTATION == 2 // 180° 倒屏
        #define X_SHIFT      0
        #define Y_SHIFT      0
    #elif LCD_ROTATION == 3 // 270° 反向横屏
        #define X_SHIFT      0
        #define Y_SHIFT      0
    #endif
#endif

#define LCD_6x12 12
#define LCD_8x16 16
#define LCD_12x24 24
#define LCD_16x32 32


#define Printf_Bcolor 0x0000;
#define Printf_Fcolor 0xFFFF;

extern const uint8_t ASCII_6x12[][12];
extern const uint8_t ASCII_8x16[][16];
extern const uint8_t ASCII_12x24[][36];
extern const uint8_t ASCII_16x32[][64]; 
extern const uint8_t TestImage[][32];


#endif


