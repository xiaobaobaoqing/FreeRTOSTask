#ifndef _BLUDETOOTH_H
#define _BLUDETOOTH_H


#include <stdarg.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdio.h>
#include "main.h"
#include <string.h>

extern volatile uint8_t Key1_Flag;
extern char Serial_RxPacket[100];
extern uint8_t Serial_RxFlag;

void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);

void Bluetooth_Init(void);
void Bluetooth_ON(void);
void Bluetooth_OFF(void);


#endif

