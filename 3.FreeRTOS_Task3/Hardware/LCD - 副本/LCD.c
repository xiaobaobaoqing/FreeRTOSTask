#include "LCD.h"




// LCD 初始化函数（LCD 完整驱动初始化序列）
void LCD_Init(void)
{
    
    SPI_GPIO_Init();                // 1. 初始化 SPI 引脚 + SPI 外设
    LCD_DMAInit();                  // 2. 初始化 DMA 引脚 + DMA 外设
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

void LCD_SetWindows(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    uint8_t buf[4];
    

    // --- 设置 X 坐标 (CASET: 0x2A) ---
    LCD_WriteCommand(LCD_CASET);
    buf[0] = xs >> 8;
    buf[1] = xs & 0xFF;
    buf[2] = xe >> 8;
    buf[3] = xe & 0xFF;
    LCD_WriteDataARR(buf, 4);

    // --- 设置 Y 坐标 (RASET: 0x2B) ---
    LCD_WriteCommand(LCD_RASET);
    buf[0] = ys >> 8;
    buf[1] = ys & 0xFF;
    buf[2] = ye >> 8;
    buf[3] = ye & 0xFF;
    LCD_WriteDataARR(buf, 4);

    // --- 准备写入显存 (RAMWR: 0x2C) ---
    LCD_WriteCommand(LCD_RAMWR);
}







//向 LCD 屏幕发送一个像素点的颜色数据
void LCD_WriteOneDot(u16 color)
{ 
    SPI_Start();
    SPI_DC(LCD_WData);
    SPI_SendByte((color >> 8) & 0xFF);
    SPI_SendByte(color & 0xFF);
    SPI_Stop();

}

void LCD_FillColorAreav(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    uint32_t w = xe - xs + 1;
    uint32_t h = ye - ys + 1;
    uint32_t total_pixel = w * h;

    LCD_SetWindows(xs, ys, w, h);

    // 直接用你写好的 DMA 填充！
    LCD_DMAFillColor(color, total_pixel);
}
//向 LCD 屏幕一块填充颜色
void LCD_FillColorAreax(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t* color)
{
    // 1. 计算宽度、高度、总像素数
    uint32_t w = xe - xs + 1;
    uint32_t h = ye - ys + 1;
    uint32_t total_pixel = w * h;


    LCD_SetWindows(xs, ys, w, h);

    // 3. 开始SPI传输
    SPI_Start();
    SPI_DC(LCD_WData);  // 写数据

    // 4. 循环DMA发送（用你现有的LCD_DMA_BUFFER_SIZE）
    while (total_pixel > 0)
    {
        uint32_t send_num;

        // 一次最多发缓冲区大小
        if (total_pixel >=LCD_DMA_BUFFER_SIZE)
        {
            send_num = LCD_DMA_BUFFER_SIZE;
        }
        else
        {
            send_num = total_pixel;
        }

        // DMA发送：颜色数组 → SPI屏幕
        LCD_DMASendBuffer((uint8_t *)color, send_num * 2);

        // 指针后移，剩余数量减少
        color += send_num;
        total_pixel -= send_num;
    }

    // 5. 结束传输
    SPI_Stop();
}





// 清屏函数（全屏填充颜色）
// 优化后的 清屏函数 (速度极快，不闪屏)
void LCD_Clear(uint16_t color)
{
    LCD_SetWindows(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LCD_DMAFillColor(color, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
}

void LCD_ClearArea(uint16_t x, uint16_t y, uint16_t font, uint16_t color)
{
    uint16_t h = 0;
    // 获取字体高度
    if(font == LCD_6x12) h = 12;
    else if(font == LCD_8x16) h = 16;
    else if(font == LCD_12x24) h = 24;
    else if(font == LCD_16x32) h = 32;

    uint16_t w = 240 - x; // 清到屏幕右侧

    // 先用 CPU 发送窗口指令 (这部分不需 DMA)
    LCD_SetWindows(x, y, w, h);

    // 再用 DMA 快速填色 (这部分是性能核心)
    LCD_DMAFillColor(color, (uint32_t)w * h);
}




// ======================== s设置屏幕旋转方向 ========================
// 参数：0 = 0°   90 = 90°   180 = 180°   270= 270°
void LCD_SetRotation(uint8_t m)
{
	LCD_WriteCommand(LCD_MADCTL);	// 发送指令：设置显示方向
	
	switch (m) {
		case 0: // 0° 竖屏（默认）
			LCD_WriteData(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
			break;
		
		case 90: // 90° 横屏
			LCD_WriteData(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
			break;
		
		case 180: // 180° 倒屏
			LCD_WriteData(MADCTL_RGB);
			break;
		
		case 270: // 270° 反向横屏
			LCD_WriteData(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
			break;
		
		default:
			break;
	}
}





void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    LCD_SetCursor(x, y); 
    LCD_WriteOneDot(color);

}
// 画线函数（Bresenham算法）
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{

    uint16_t swap;
    uint16_t steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep)
    {
        swap = x0; x0 = y0; y0 = swap;
        swap = x1; x1 = y1; y1 = swap;
    }

    if (x0 > x1)
    {
        swap = x0; x0 = x1; x1 = swap;
        swap = y0; y0 = y1; y1 = swap;
    }

    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++)
    {
        if (steep)
            LCD_DrawPixel(y0, x0, color);
        else
            LCD_DrawPixel(x0, y0, color);

        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}
//画空心矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}
//画空心圆
void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    LCD_DrawPixel(x0, y0 + r, color);
    LCD_DrawPixel(x0, y0 - r, color);
    LCD_DrawPixel(x0 + r, y0, color);
    LCD_DrawPixel(x0 - r, y0, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        LCD_DrawPixel(x0 + x, y0 + y, color);
        LCD_DrawPixel(x0 - x, y0 + y, color);
        LCD_DrawPixel(x0 + x, y0 - y, color);
        LCD_DrawPixel(x0 - x, y0 - y, color);

        LCD_DrawPixel(x0 + y, y0 + x, color);
        LCD_DrawPixel(x0 - y, y0 + x, color);
        LCD_DrawPixel(x0 + y, y0 - x, color);
        LCD_DrawPixel(x0 - y, y0 - x, color);
    }
}
//显示图片
void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data)
{
    uint16_t dx, dy;
    uint16_t px, py;

    for (dy = 0; dy < h; dy++)
    {
        // Y 环绕：超出屏幕高度 → 回到顶部
        py = y + dy;
        if (py >= LCD_HEIGHT) py -= LCD_HEIGHT;

        for (dx = 0; dx < w; dx++)
        {
            // X 环绕：超出屏幕宽度 → 回到左侧
            px = x + dx;
            if (px >= LCD_WIDTH) px -= LCD_WIDTH;

            // 取颜色
            uint16_t color = (data[(dy * w + dx) * 2] << 8) | data[(dy * w + dx) * 2 + 1];

            // 画点
            LCD_DrawPixel(px, py, color);
        }
    }
}






// ===================== 显示单个字符 ===================== 
// 参考你的 TFT_ShowChar 风格 → 适配你的 16*16 字库
// 16×16 字符显示（
// 逐行式 16×16 
// mode = 0 → 不透明（带背景色）
// mode = 1 → 透明（只写字，不画背景）
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t size, char c, uint16_t color, uint16_t bcolor, uint8_t mode)
{
    
    uint8_t *pfont = NULL;
    uint8_t temp;
    uint8_t csize = (size/8+((size%8)?1:0))*(size/2); // 字符字节数
    uint16_t y0 = y; // 保存起始Y坐标
    c = c - ' '; // 字库偏移

    // 匹配字体（完全和你原来一样）
    switch(size)
    {
        case 12:  pfont = (uint8_t *)ASCII_6x12[(uint8_t)c]; break;
        case 16:  pfont = (uint8_t *)ASCII_8x16[(uint8_t)c]; break;
        case 24:  pfont = (uint8_t *)ASCII_12x24[(uint8_t)c]; break;
        case 32:  pfont = (uint8_t *)ASCII_16x32[(uint8_t)c]; break;
        default:  return;
    }

    // 完全保留你原来的绘制逻辑！！！
    for(uint8_t t=0; t<csize; )
    {
        for(uint16_t yy = 0; yy < size; yy++)
        {
            if(yy % 8 == 0)
            {
                temp = pfont[t++];
            }

            // ======================
            // 核心：加入 mode 判断
            // ======================
            if(temp & (0x01 << (yy % 8)))
            {
                // 画字体
                LCD_DrawPixel(x, y0 + yy, color);
            }
            else
            {
                // mode=0 才画背景色
                if(mode == 0)
                {
                    LCD_DrawPixel(x, y0 + yy, bcolor);
                }
            }
        }
        x++;
    }
}
// 画字符串函数
void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size ,const char *str, uint16_t color,uint16_t bcolor, uint8_t mode)
{   
    if(x >= LCD_WIDTH || y >= LCD_HEIGHT) return; // 起始点超出屏幕范围，不绘制

    uint16_t char_width = size / 2;  // 字符宽度
    uint16_t draw_x = x;
    uint16_t draw_y = y;
    while(*str)
    {
        // ======================
        // 核心：X Y 双向环绕计算
        // ======================
        uint16_t real_x = draw_x % LCD_WIDTH;
        uint16_t real_y = draw_y % LCD_HEIGHT;
        
        // 画字符（用环绕后的坐标）
        LCD_ShowChar(real_x, real_y, size, *str, color, bcolor, mode);

        // 移动坐标
        draw_x += char_width;

        // 自动换行 + 环绕
        if(draw_x >= LCD_WIDTH) 
        {
            draw_x -= LCD_WIDTH;  // X环绕：从左边继续
            draw_y += size;       // 换行
            draw_y %= LCD_HEIGHT; // Y环绕：超出就回到顶部
        }

        str++;
    }
   

}
// 显示十进制无符号数字
void LCD_ShowNum(uint16_t x, uint16_t y, uint8_t size, uint32_t Num, uint8_t Len)
{
    uint8_t i;
    char buf[10] = {0};
    
    // 数字转字符（低位在前）
    for (i = 0; i < Len; i++)
    {
        buf[Len - 1 - i] = Num % 10 + '0';
        Num /= 10;
    }
    
    // 显示字符串（透明模式0，带背景）
    LCD_ShowString(x, y, size, buf, WHITE, BLACK, 0);
}

// 显示有符号数字（支持负数）
void LCD_ShowSignedNum(uint16_t x, uint16_t y, uint8_t size, int32_t Num, uint8_t Len)
{
    uint8_t i;
    char buf[10] = {0};
    uint8_t flag = 0;

    // 负数处理
    if (Num < 0)
    {
        flag = 1;
        Num = -Num;
    }

    // 数字转字符
    for (i = 0; i < Len; i++)
    {
        buf[Len - 1 - i] = Num % 10 + '0';
        Num /= 10;
    }

    // 负号填入
    if (flag) buf[0] = '-';

    LCD_ShowString(x, y, size, buf, WHITE, BLACK, 0);
}

// 显示十六进制数字
void LCD_ShowHexNum(uint16_t x, uint16_t y, uint8_t size, uint32_t Num, uint8_t Len)
{
    uint8_t i, temp;
    char buf[10] = {0};

    for (i = 0; i < Len; i++)
    {
        temp = Num % 16;
        if (temp < 10) buf[Len - 1 - i] = temp + '0';
        else           buf[Len - 1 - i] = temp - 10 + 'A';
        Num /= 16;
    }

    LCD_ShowString(x, y, size, buf, WHITE, BLACK, 0);
}

void LCD_Printf(uint16_t x, uint16_t y, uint8_t size,const char *format, ...)
{
    switch(size)
    {   
        case LCD_6x12:  
            if(y>=23)y=23;
            y=12*(y-1)+Y_SHIFT;
            break;
        case LCD_8x16:  
            if(y>=17)y=17;
            x=(x-1)*8+20;
            y=16*(y-1)+Y_SHIFT;
            break;
        case LCD_12x24:
            y=24*(y-1)+Y_SHIFT;
            break;
        case LCD_16x32:
            y=32*(y-1)+Y_SHIFT;
            break;
    }


    char buffer[128]; // 临时字符串缓冲区
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args); // 格式化字符串
    va_end(args);

    LCD_ShowString(x, y, size, buffer, WHITE, BLACK, 0);
}



