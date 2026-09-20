#include "LCD.h"




// LCD 初始化函数（LCD 完整驱动初始化序列）
void LCD_Init(void)
{
    
    SPI_GPIO_Init();                // 1. 初始化 SPI 引脚 + SPI 外设
    // LCD_DMAInit();                  // 2. 初始化 DMA 引脚 + DMA 外设
    LCD_Reset();                    // 2. LCD 硬件复位（必须复位，屏幕才能进入就绪状态）

    // ===================== LCD 核心初始化配置 =====================
    // 3. 设置颜色格式：0x3A=COLMOD，0x55=RGB565（16位色，最常用）
    LCD_WriteCommand(LCD_COLMOD);  
    LCD_WriteData(0x55);

    // 4. 设置显示方向：0x36=MADCTL，0x00=默认竖屏、不翻转
    LCD_WriteCommand(LCD_MADCTL);  
    LCD_WriteData(0x00);
    
    // 5. 设置帧速率：0xC6，0x0F=标准刷新率配置
    LCD_WriteCommand(LCD_FRAMERATE);  
    LCD_WriteData(0x0F);
    
    // 6. 开启反色显示（部分屏幕需要，可增强对比度）
    LCD_WriteCommand(LCD_INVON);   
    
    // 7. 设置电源控制 B7：0x35=调节栅极电压
    LCD_WriteCommand(LCD_GATE_CONTROL); 
    LCD_WriteData(0x35);
    
    // 8. 设置电源控制 BB：0x19=调节VCOM电压
    LCD_WriteCommand(LCD_VCMOS_SET); 
    LCD_WriteData(0x19);
    
    // 9. 电源控制 C0：设置LCM电压
    LCD_WriteCommand(LCD_LCMCTRL); 
    LCD_WriteData(0x2C);
    
    // 10. 电源控制 C2：DC/CL 开启
    LCD_WriteCommand(LCD_VDVVRHEN); 
    LCD_WriteData(0x01);
    
    // 11. 电源控制 C3：调节升压电压
    LCD_WriteCommand(LCD_VRHSET); 
    LCD_WriteData(0x12);
    
    // 12. 电源控制 C4：调节VDV电压
    LCD_WriteCommand(LCD_VDVSET); 
    LCD_WriteData(0x20);

    // 13. 电源开启控制 D0：设置电源模式
    LCD_WriteCommand(LCD_POWER_CTR1);
    LCD_WriteData(0xA4);
    LCD_WriteData(0xA1);

    // 14. 帧控制 + 消影控制 B2：配置显示时序
    LCD_WriteCommand(LCD_PORCH_SET);
    {
        uint8_t Data[] = {0x0C,0x0C,0x00,0x33,0x33};
        LCD_WriteDataARR(Data,sizeof(Data));
    }

    // 15. 正极性伽马校正 E0：调节红色调
    LCD_WriteCommand(LCD_PVGAM_CTRL);
    {
        uint8_t Data[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
        LCD_WriteDataARR(Data,sizeof(Data));
    }

    // 16. 负极性伽马校正 E1：调节蓝色调
    LCD_WriteCommand(LCD_NVGAM_CTRL);
    {
        uint8_t Data[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
        LCD_WriteDataARR(Data,sizeof(Data));
    }

    // ===================== 最后启动显示 =====================
    // 17. 退出睡眠模式（必须，否则屏幕不工作）
    LCD_WriteCommand(LCD_SLPOUT);
    
    // 18. 打开正常显示模式
    LCD_WriteCommand(LCD_NORON);
    
    // 19. 开启显示（屏幕最终点亮）
    LCD_WriteCommand(LCD_DISPON); 
    
    LCD_Clear(BLACK); 
}

void LCD_Reset(void)
{
    GPIO_WriteBit(GPIOB, LCD_RST, Bit_RESET);
    Delay_ms(10);
    GPIO_WriteBit(GPIOB, LCD_RST, Bit_SET);
    Delay_ms(10);
}





void LCD_SetCursor(uint16_t x, uint16_t y)
{
    uint8_t buf[4];  // 用来存放地址参数
    uint16_t phys_y=y+Y_SHIFT;  // 计算物理坐标（加上偏移量）

    // ========== 设置 X 列地址 0x2A ==========
    buf[0] = (x >> 8) & 0xFF;
    buf[1] = x & 0xFF;
    buf[2] = buf[0];  // 结束地址 = 开始地址
    buf[3] = buf[1];
    
    LCD_WriteCommand(LCD_CASET);
    LCD_WriteDataARR(buf, 4);  

    // ========== 设置 Y 行地址 0x2B ==========
    buf[0] = (phys_y >> 8) & 0xFF;
    buf[1] = phys_y & 0xFF;
    buf[2] = buf[0];  // 结束地址 = 开始地址
    buf[3] = buf[1];
    
    LCD_WriteCommand(LCD_RASET);
    LCD_WriteDataARR(buf, 4);  // 一次性发4个字节
    // ========== 写入颜色指令 0x2C ==========
    LCD_WriteCommand(LCD_RAMWR);
}

void LCD_SetWindows(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint8_t buf[4];
    
    // 1. 计算结束坐标 (注意：坐标是从 0 开始的，所以要减 1)
    uint16_t x_start = x;
    uint16_t x_end   = x + w - 1;
    
    // 2. 计算 Y 轴偏移 (针对 240x280 屏幕)
    uint16_t y_start = y + Y_SHIFT;
    uint16_t y_end   = y + h - 1 + Y_SHIFT;

    // --- 设置 X 坐标 (CASET: 0x2A) ---
    LCD_WriteCommand(LCD_CASET);
    buf[0] = x_start >> 8;
    buf[1] = x_start & 0xFF;
    buf[2] = x_end >> 8;
    buf[3] = x_end & 0xFF;
    LCD_WriteDataARR(buf, 4);

    // --- 设置 Y 坐标 (RASET: 0x2B) ---
    LCD_WriteCommand(LCD_RASET);
    buf[0] = y_start >> 8;
    buf[1] = y_start & 0xFF;
    buf[2] = y_end >> 8;
    buf[3] = y_end & 0xFF;
    LCD_WriteDataARR(buf, 4);

    // --- 准备写入显存 (RAMWR: 0x2C) ---
    LCD_WriteCommand(LCD_RAMWR);
}







// 清屏函数（全屏填充颜色）
// 优化后的 清屏函数 (速度极快，不闪屏)
void LCD_Clear(uint16_t color)
{
    LCD_SetWindows(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LCD_DMAFillColor(color, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
}


