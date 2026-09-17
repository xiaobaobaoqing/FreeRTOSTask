#ifndef LCD_H_
#define LCD_H_

  /* 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~239
  * 纵向向下为Y轴，取值范围：0~319
  * 
  *       0             X轴           239 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   319 |
  *      v
  * 
  */

#include "stm32f4xx.h"
#include "Delay.h"
#include "LCD_Define.h"
#include "LCD_Hardware.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"

void LCD_Init(void);
void LCD_Reset(void);
void LCD_Clear(uint16_t color);
void LCD_ClearArea(uint16_t x, uint16_t y, uint16_t font, uint16_t color);

void LCD_SetCursor(uint16_t x, uint16_t y);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);
void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data);


void LCD_FillColorAreax(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t* color);
void LCD_FillColorAreav(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t size, char c, uint16_t color, uint16_t bcolor, uint8_t mode);
void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size ,const char *str, uint16_t color,uint16_t bcolor, uint8_t mode);
void LCD_ShowNum(uint16_t x, uint16_t y, uint8_t size, uint32_t Num, uint8_t Len);
void LCD_ShowSignedNum(uint16_t x, uint16_t y, uint8_t size, int32_t Num, uint8_t Len);
void LCD_ShowHexNum(uint16_t x, uint16_t y, uint8_t size, uint32_t Num, uint8_t Len);

void LCD_Printf(uint16_t x, uint16_t y, uint8_t size,const char *format, ...);

#endif /* LCD_H_ */

