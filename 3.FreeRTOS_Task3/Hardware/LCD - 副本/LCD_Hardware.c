#include "LCD_Hardware.h"





//片选
void SPI_CS(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOB, LCD_CS, (BitAction)BitValue);
}


//数据/命令选择
void SPI_DC(uint8_t BitValue)
{
    GPIO_WriteBit(GPIOB, LCD_DC, (BitAction)BitValue);
}


//SPI片选开始
void SPI_Start(void)
{
    SPI_CS(0);
}


//SPI片选结束
void SPI_Stop(void)
{
    SPI_CS(1);
}


//SPI发送一个字节
void SPI_SendByte(uint8_t Data)
{

   while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=SET);
   SPI_I2S_SendData(SPI1,Data);
   while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

}


//发送命令
void LCD_WriteCommand(uint8_t Command)
{
    SPI_Start();
    SPI_DC(LCD_WCommand);
    SPI_SendByte(Command);
    SPI_Stop();
}


//发送数据
void LCD_WriteData(uint8_t Data)
{
    SPI_Start();
    SPI_DC(LCD_WData);
    SPI_SendByte(Data);
    SPI_Stop();

}


// 发送数据数组
void LCD_WriteDataARR(uint8_t* Data,uint16_t Len)
{
    SPI_Start();
    SPI_DC(LCD_WData);
    // 循环发送每一个字节
    for(uint16_t i=0; i<Len; i++)
    {
        // 等待发送缓冲区为空
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        
        // 发送当前字节
        SPI_I2S_SendData(SPI1, *Data++);
    }

    // 所有字节发完后，等待 SPI 硬件完全结束
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

    SPI_Stop();

}


void SPI_GPIO_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin  = LCD_CS|LCD_DC|LCD_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin  = LCD_MOSI|LCD_CLK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);


    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2; //2分频 100Mhz/2=50Mhz
    SPI_InitStructure.SPI_CPHA=SPI_CPOL_High;                         //串行时钟的空闲状态为低电平
    SPI_InitStructure.SPI_CPOL=SPI_CPHA_2Edge;                       //串行时钟的上升沿为采样时钟的第一个边沿
    SPI_InitStructure.SPI_CRCPolynomial=7;                        //CRC校验位                   
    SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;                  //数据大小为8位
    SPI_InitStructure.SPI_Direction=SPI_Direction_1Line_Tx;         //模式1，单向数据传输
    SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;                //数据传输从MSB位开始
    SPI_InitStructure.SPI_Mode=SPI_Mode_Master;                     //主机模式
    SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;                          //NSS信号由软件管理


    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
    
    SPI_CS(1);
}














//DMA2_Stream3数据流 是SPI1的TX DMA，
void LCD_DMAInit(void)
{
    DMA_InitTypeDef DMA_InitStruct;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DMA_Cmd(LCD_DMA_STREAM, DISABLE);                                        //先关闭DMA，防止正在运行
    while(DMA_GetCmdStatus(LCD_DMA_STREAM) != DISABLE);                      //等待DMA关闭               

    DMA_DeInit(LCD_DMA_STREAM);


    // ===================== 核心配置：内存→外设搬运 =====================
    DMA_InitStruct.DMA_Channel = LCD_DMA_CHANNEL;       
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;            // 目标地址：SPI的数据寄存器（发给屏幕）
    DMA_InitStruct.DMA_Memory0BaseAddr =0;                                  // 源地址：等下会动态改，这里先填0
    DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;                    // 方向：从 内存(数组) 搬到 外设(SPI屏幕)
    DMA_InitStruct.DMA_BufferSize = 0;                                      // 数据长度：等下发多少改多少，先写0

    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;           // 外设地址不变（一直发给SPI同一个寄存器）
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;                    // 内存地址要自增（数组一个一个往后读）


    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;   // 1字节
    DMA_InitStruct.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;       // 1字节

    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;         // 搬完就停
    DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh; // 优先级中等

   // 4. FIFO 配置 (SPI 字节流建议禁用 FIFO 提升效率)
    DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    // 3. 初始化DMA
    DMA_Init(LCD_DMA_STREAM, &DMA_InitStruct);
    
    // 打开SPI的DMA发送功能（SPI一有空，DMA就自动发数据）
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
}


void LCD_DMASendBuffer(uint8_t *buf,uint32_t len)
{

    DMA_Cmd(LCD_DMA_STREAM, DISABLE);                           //先关闭DMA，防止正在运行
    while(DMA_GetCmdStatus(LCD_DMA_STREAM) != DISABLE);         //等待DMA关闭               

    // 清空DMA的所有标志（防止上次干扰这次）
    DMA_ClearFlag(LCD_DMA_STREAM,DMA_FLAG_TCIF3);
    

    DMA_SetCurrDataCounter(LCD_DMA_STREAM,len);                         // 设置数据长度
    DMA_MemoryTargetConfig(LCD_DMA_STREAM,(uint32_t)buf,DMA_Memory_0);  // 设置源地址


    DMA_Cmd(LCD_DMA_STREAM, ENABLE);                                     // 开启DMA

    while(DMA_GetFlagStatus(LCD_DMA_STREAM,DMA_FLAG_TCIF3) == RESET);    // 等待DMA搬运完成

    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) == SET);          // 等待SPI空闲
}


void LCD_DMAFillColor(uint16_t color, uint32_t pixel_count)
{

    if(pixel_count > LCD_WIDTH*LCD_HEIGHT) pixel_count = LCD_WIDTH*LCD_HEIGHT;

    uint32_t send_count;
    uint16_t i;

    uint8_t high = color >> 8;
    uint8_t low  = color & 0xFF;
    //==================================================
    // 初始化颜色缓存
    //==================================================
    static uint16_t last_color = 0xFFFF;
    if(color != last_color)
     {
            for(i = 0; i < LCD_DMA_BUFFER_SIZE; i++)
            {
                LCD_DMA_ColorBuffer[i * 2]     = high;
                LCD_DMA_ColorBuffer[i * 2 + 1] = low;
            }
            last_color = color;
     }
    //==================================================
    // DMA循环发送
    //==================================================
    SPI_Start();                                                // 拉低 CS (使用你代码里的宏/函数)
    SPI_DC(LCD_WData);                                          // 拉高 DC (告诉屏幕现在发的是颜色数据)
    while(pixel_count > 0)
    {
        //----------------------------------------------
        // 计算本次发送数量
        //----------------------------------------------
        if(pixel_count >= LCD_DMA_BUFFER_SIZE)
        {
            send_count = LCD_DMA_BUFFER_SIZE;
        }
        else
        {
            send_count = pixel_count;
        }

        //----------------------------------------------
        // DMA发送颜色块
        //----------------------------------------------
        LCD_DMASendBuffer(LCD_DMA_ColorBuffer,send_count*2);

        //----------------------------------------------
        // 更新剩余像素数量
        //----------------------------------------------
        pixel_count -= send_count;
    }
    SPI_Stop();                                                          // 拉高 CS       
}
