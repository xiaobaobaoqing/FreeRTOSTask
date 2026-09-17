#ifndef LCD_HARMWARE_H_
#define LCD_HARMWARE_H_

#include "stm32f4xx.h"
#include "LCD_Define.h"



#define LCD_BLK GPIO_Pin_0  
#define LCD_CLK GPIO_Pin_3
#define LCD_MOSI GPIO_Pin_5     
#define LCD_RST GPIO_Pin_7  //复位
#define LCD_CS  GPIO_Pin_8  //片选信号
#define LCD_DC  GPIO_Pin_9  //数据/命令选择脚，高电平表示数据，低电平表示命令


#define LCD_WCommand 0
#define LCD_WData    1


void SPI_CS(uint8_t BitValue);
void SPI_DC(uint8_t BitValue);

void SPI_Start(void);
void SPI_Stop(void);
void SPI_SendByte(uint8_t Data);

void LCD_WriteCommand(uint8_t Command);
void LCD_WriteData(uint8_t Data);
void LCD_WriteDataARR(uint8_t* Data,uint16_t Len);
void SPI_GPIO_Init(void);





#define LCD_DMA_BUFFER_SIZE    256
static uint8_t LCD_DMA_ColorBuffer[LCD_DMA_BUFFER_SIZE*2];


#define LCD_DMA_STREAM     DMA2_Stream3
#define LCD_DMA_CHANNEL    DMA_Channel_3


void LCD_DMAInit(void);

void LCD_DMASendBuffer(uint8_t *buf,uint32_t len);

void LCD_DMAFillColor(uint16_t color,uint32_t pixel_count);

#endif /* LCD_DMA_H_ */


