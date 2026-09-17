#include "LCD.h"
#include "stm32f4xx_hal.h"
#include "main.h"

static uint16_t lcd_dma_chunk_buf[128];
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

typedef void (*LCD_CallbackFunc_t)(void);
static LCD_CallbackFunc_t lcd_ready_cb = NULL;

static void SPI_CS(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, LCD_CS_PIN, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void SPI_DC(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, LCD_DC_PIN, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void SPI_Start(void)
{
    SPI_CS(0);
}

static void SPI_Stop(void)
{
    SPI_CS(1);
}

static void SPI_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    GPIO_InitStruct.Pin = LCD_CS_PIN | LCD_DC_PIN | LCD_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, LCD_CS_PIN | LCD_DC_PIN | LCD_RST_PIN, GPIO_PIN_SET);


    GPIO_InitStruct.Pin = LCD_CLK_PIN|LCD_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_1LINE;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.BaudRatePrescaler =  SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation =SPI_CRCCALCULATION_DISABLE;

    hspi1.Init.CRCPolynomial = 7;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }

    
    SPI_CS(1);
}

static void LCD_DMAInit(void)
{


    __HAL_RCC_DMA2_CLK_ENABLE();

    hdma_spi1_tx.Instance = DMA2_Stream3;
    hdma_spi1_tx.Init.Channel  = DMA_CHANNEL_3;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

        if(HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
    {
        Error_Handler();
    }
    //把DMA句柄绑定到SPI1的TxDMA
    __HAL_LINKDMA(&hspi1,hdmatx,hdma_spi1_tx);

    // NVIC 配置
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
 * @brief 注册DMA传输完成回调
 */
void LCD_Set_Flush_Complete_Callback(LCD_CallbackFunc_t cb)
{
	lcd_ready_cb = cb;
}

// SPI DMA传输完成回调，HAL库标准回调
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		// 确保最后一位数据离开 STM32 的移位寄存器
		while (hspi->Instance->SR & SPI_FLAG_BSY);
		if (lcd_ready_cb)
		{
			lcd_ready_cb();
		}
	}
}



static void LCD_RESET(void)
{
    HAL_GPIO_WritePin(GPIOB, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOB, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
}

static void LCD_WriteCommand(uint8_t Command)
{
    SPI_DC(0);
    HAL_SPI_Transmit(&hspi1, &Command, 1, HAL_MAX_DELAY);
    SPI_DC(1);

}

static void LCD_WriteData(uint8_t Data)
{
    HAL_SPI_Transmit(&hspi1, &Data, 1, HAL_MAX_DELAY);
}

static void LCD_WriteDataARR(uint8_t *pData, uint16_t Length)
{
    HAL_SPI_Transmit(&hspi1,pData,Length,1000);
}


// LCD 初始化函数（LCD 完整驱动初始化序列）
void LCD_Init(void)
{
    
    SPI_GPIO_Init();                // 1. 初始化 SPI 引脚 + SPI 外设
     LCD_DMAInit();                  // 2. 初始化 DMA 引脚 + DMA 外设
    LCD_RESET();                    // 2. LCD 硬件复位（必须复位，屏幕才能进入就绪状态）

    // ===================== LCD 核心初始化配置 =====================
    // 3. 设置颜色格式：0x3A=COLMOD，0x55=RGB565（16位色，最常用）
    SPI_Start();

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
    HAL_Delay(120);
    // 18. 打开正常显示模式
    LCD_WriteCommand(LCD_NORON);
    
    // 19. 开启显示（屏幕最终点亮）
    LCD_WriteCommand(LCD_DISPON); 

    SPI_Stop();

    LCD_Cleard(BLACK);  // 清屏


}


static void LCD_SetCursor(uint16_t x, uint16_t y)
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

static void LCD_SetWindows(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    uint8_t buf[4];
    xe -= 1; // 调整 X 坐标
    ye -= 1; // 调整 Y 坐标
    ye += Y_SHIFT; // 调整 Y 坐标
    ys += Y_SHIFT; // 调整 Y 坐标

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

void LCD_Clears(uint16_t color)
{
    LCD_FillColorAreas(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void LCD_Cleard(uint16_t color)
{
      LCD_FillColorAread(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, color);

}

void LCD_FillColorAreas(uint16_t xs , uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
 
    uint16_t width  = xe - xs + 1;
    uint16_t height = ye - ys + 1;
    uint32_t total  = (uint32_t)width * height;

    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;

    static uint8_t buf[128 * 2];
    for (int i = 0; i < 128; i++)
    {
        buf[2*i] = hi;
        buf[2*i + 1] = lo;
    }

    SPI_Start();
    LCD_SetWindows(xs, ys, xe, ye);
    SPI_DC(1);

    while (total > 0)
    {
        uint16_t n = (total > 128) ? 128 : (uint16_t)total;
        HAL_SPI_Transmit(&hspi1, buf, n * 2, HAL_MAX_DELAY);
        total -= n;
    }
    SPI_Stop();
}

void LCD_FillColorAread(uint16_t xs , uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
        uint32_t total_pixel = (uint32_t)(xe - xs + 1U) * (uint32_t)(ye - ys + 1U);
        if (total_pixel == 0U) return;

        // 静态颜色缓冲区（一次发 128 像素 = 256 字节）
        static uint16_t color_buf[256];
        static uint16_t last_color = 0xFFFF;
        static uint8_t  buf_valid  = 0;

            /* 只有颜色变了才重填，清屏 / 大面积填充同一颜色时省 CPU */

        if (!buf_valid || color != last_color)
        {
            uint16_t swp = __REV16(color);              /* 等价于 (color>>8)|(color<<8) */
            for (uint32_t i = 0; i < 256U; i++)
            {
                color_buf[i] = swp;
            }
            last_color = color;
            buf_valid  = 1U;
        }

        SPI_Start();
        
        LCD_SetWindows(xs, ys, xe + 1U, ye + 1U);

        SPI_DC(1);                                       

        /* ---------- 4. 分块循环 DMA 发送 ---------- */
        uint32_t sent = 0;
        while (sent < total_pixel)
        {
            uint32_t n = (total_pixel - sent > 256U)  ? 256U : (total_pixel - sent);

            while (hspi1.State == HAL_SPI_STATE_BUSY_TX) { }

            HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)color_buf, n * 2U);
            sent += n;
        }

        /* ---------- 5. 收尾：等最后一次 DMA + 移位寄存器清空 ---------- */
        while (hspi1.State == HAL_SPI_STATE_BUSY_TX) { }
        while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET) { }

        SPI_Stop();
}

void LCD_Draw(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;
    LCD_SetCursor(x, y);
    LCD_WriteData(hi);
    LCD_WriteData(lo);
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    SPI_Start();
    LCD_Draw(x, y, color);
    SPI_Stop();
}

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

    SPI_Start();
    for (; x0 <= x1; x0++)
    {
        if (steep)
            LCD_Draw(y0, x0, color);
        else
            LCD_Draw(x0, y0, color);

        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
    SPI_Stop();
}

void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    SPI_Start();

    LCD_Draw(x0, y0 + r, color);
    LCD_Draw(x0, y0 - r, color);
    LCD_Draw(x0 + r, y0, color);
    LCD_Draw(x0 - r, y0, color);

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

        LCD_Draw(x0 + x, y0 + y, color);
        LCD_Draw(x0 - x, y0 + y, color);
        LCD_Draw(x0 + x, y0 - y, color);
        LCD_Draw(x0 - x, y0 - y, color);

        LCD_Draw(x0 + y, y0 + x, color);
        LCD_Draw(x0 - y, y0 + x, color);
        LCD_Draw(x0 + y, y0 - x, color);
        LCD_Draw(x0 - y, y0 - x, color);
    }

    SPI_Stop();
}

void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data)
{
    uint16_t dx, dy;
    uint16_t px, py;

    SPI_Start();
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
            LCD_Draw(px, py, color);
        }
    }
    SPI_Stop();
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

    SPI_Start();
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
                LCD_Draw(x, y0 + yy, color);
            }
            else
            {
                // mode=0 才画背景色
                if(mode == 0)
                {
                    LCD_Draw(x, y0 + yy, bcolor);
                }
            }
        }
        x++;
    }
    SPI_Stop();
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


void LCD_LED(uint16_t x, uint16_t y, uint16_t color)
{
    uint16_t xe = x + 20 - 1U;
    uint16_t ye = y + 20 - 1U;
    LCD_FillColorAread(x, y, xe, ye, color);
}