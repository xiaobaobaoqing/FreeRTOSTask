#ifndef __TOUCH_I2C_H
#define __TOUCH_I2C_H

#include "Touch.h"

void CST816D_I2C_Init(void);
void CST816D_I2C_Start(void);
void CST816D_I2C_Stop(void);
void CST816D_I2C_SendByte(uint8_t Byte);
uint8_t CST816D_I2C_ReceiveByte(void);
void CST816D_I2C_SendAck(uint8_t AckBit);
uint8_t CST816D_I2C_ReceiveAck(void);


void CST816D_I2C_WriteReg(uint8_t Reg, uint8_t Data);	
uint8_t CST816D_I2C_ReadReg(uint8_t Reg);
void CST816D_I2C_WriteArray(uint8_t Reg, uint8_t* Data, uint16_t Len);
void CST816D_I2C_ReadArray(uint8_t Reg, uint8_t* Data, uint16_t Len);


#endif
