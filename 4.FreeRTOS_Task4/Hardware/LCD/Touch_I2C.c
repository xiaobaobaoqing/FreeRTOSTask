#include "Touch_I2C.h"
/*引脚配置层*/

/**
  * 函    数：I2C写CST816D_I2C_SCL引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入CST816D_I2C_SCL的电平，范围0~1
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置CST816D_I2C_SCL为低电平，当BitValue为1时，需要置CST816D_I2C_SCL为高电平
  */
void CST816D_I2C_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(CST816D_IIC_GPIO_Port, CST816D_SCL, (BitAction)BitValue);		//根据BitValue，设置CST816D_I2C_SCL引脚的电平
	Delay_us(1);	//适当延时，确保电平稳定
}

/**
  * 函    数：I2C写CST816D_I2C_SDA引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入CST816D_I2C_SDA的电平，范围0~0xFF
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，当BitValue为0时，需要置CST816D_I2C_SDA为低电平，当BitValue非0时，需要置CST816D_I2C_SDA为高电平
  */
void CST816D_I2C_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(CST816D_IIC_GPIO_Port, CST816D_SDA, (BitAction)BitValue);		//根据BitValue，设置CST816D_I2C_SDA引脚的电平，BitValue要实现非0即1的特性
	Delay_us(1);	//适当延时，确保电平稳定

}

/**
  * 函    数：I2C读CST816D_I2C_SDA引脚电平
  * 参    数：无
  * 返 回 值：协议层需要得到的当前CST816D_I2C_SDA的电平，范围0~1
  * 注意事项：此函数需要用户实现内容，当前CST816D_I2C_SDA为低电平时，返回0，当前CST816D_I2C_SDA为高电平时，返回1
  */
uint8_t R_CST816D_I2C_SDA(void)
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(CST816D_IIC_GPIO_Port, CST816D_SDA);		//读取CST816D_I2C_SDA电平
	Delay_us(1);	//适当延时，确保电平稳定
	return BitValue;											//返回CST816D_I2C_SDA电平
}

/**
  * 函    数：I2C初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数需要用户实现内容，实现CST816D_I2C_SCL和CST816D_I2C_SDA引脚的初始化
  */
void CST816D_I2C_Init(void)
{
	/*开启时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	//开启CST816D_IIC_GPIO_Port的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//开启CST816D_IIC_GPIO_Port的时钟
	GPIO_SetBits(CST816D_RST_GPIO_Port, CST816D_RST);
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//推挽输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;			//开漏输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉电阻
	GPIO_InitStructure.GPIO_Pin = CST816D_SCL | CST816D_SDA ;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(CST816D_IIC_GPIO_Port, &GPIO_InitStructure);				


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//推挽输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉电阻
	GPIO_InitStructure.GPIO_Pin = CST816D_RST ;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(CST816D_RST_GPIO_Port, &GPIO_InitStructure);					
	/*设置默认电平*/
	GPIO_SetBits(CST816D_RST_GPIO_Port, CST816D_RST);						
	GPIO_SetBits(CST816D_IIC_GPIO_Port, CST816D_SCL | CST816D_SDA);			
}

/*协议层*/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void CST816D_I2C_Start(void)
{
	CST816D_I2C_SDA(1);							//释放CST816D_I2C_SDA，确保CST816D_I2C_SDA为高电平
	CST816D_I2C_SCL(1);							//释放CST816D_I2C_SCL，确保CST816D_I2C_SCL为高电平
	CST816D_I2C_SDA(0);							//在CST816D_I2C_SCL高电平期间，拉低CST816D_I2C_SDA，产生起始信号
	CST816D_I2C_SCL(0);							//起始后把CST816D_I2C_SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void CST816D_I2C_Stop(void)
{
	CST816D_I2C_SDA(0);							//拉低CST816D_I2C_SDA，确保CST816D_I2C_SDA为低电平
	CST816D_I2C_SCL(1);							//释放CST816D_I2C_SCL，使CST816D_I2C_SCL呈现高电平
	CST816D_I2C_SDA(1);							//在CST816D_I2C_SCL高电平期间，释放CST816D_I2C_SDA，产生终止信号
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void CST816D_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i ++)				//循环8次，主机依次发送数据的每一位
	{
		CST816D_I2C_SDA(Byte & (0x80 >> i));	//使用掩码的方式取出Byte的指定一位数据并写入到CST816D_I2C_SDA线
		CST816D_I2C_SCL(1);						//释放CST816D_I2C_SCL，从机在CST816D_I2C_SCL高电平期间读取CST816D_I2C_SDA
		CST816D_I2C_SCL(0);						//拉低CST816D_I2C_SCL，主机开始发送下一位数据
	}
}

/**
  * 函    数：I2C接收一个字节
  * 参    数：无
  * 返 回 值：接收到的一个字节数据，范围：0x00~0xFF
  */
uint8_t CST816D_I2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;					//定义接收的数据，并赋初值0x00，此处必须赋初值0x00，后面会用到
	CST816D_I2C_SDA(1);							//接收前，主机先确保释放CST816D_I2C_SDA，避免干扰从机的数据发送
	for (i = 0; i < 8; i ++)				//循环8次，主机依次接收数据的每一位
	{
		CST816D_I2C_SCL(1);						//释放CST816D_I2C_SCL，主机机在CST816D_I2C_SCL高电平期间读取CST816D_I2C_SDA
		if (R_CST816D_I2C_SDA() == 1){Byte |= (0x80 >> i);}	//读取CST816D_I2C_SDA数据，并存储到Byte变量
														//当CST816D_I2C_SDA为1时，置变量指定位为1，当CST816D_I2C_SDA为0时，不做处理，指定位为默认的初值0
		CST816D_I2C_SCL(0);						//拉低CST816D_I2C_SCL，从机在CST816D_I2C_SCL低电平期间写入CST816D_I2C_SDA
	}
	return Byte;							//返回接收到的一个字节数据
}

/**
  * 函    数：I2C发送应答位
  * 参    数：Byte 要发送的应答位，范围：0~1，0表示应答，1表示非应答
  * 返 回 值：无
  */
void CST816D_I2C_SendAck(uint8_t AckBit)
{
	CST816D_I2C_SDA(AckBit);					//主机把应答位数据放到CST816D_I2C_SDA线
	CST816D_I2C_SCL(1);							//释放CST816D_I2C_SCL，从机在CST816D_I2C_SCL高电平期间，读取应答位
	CST816D_I2C_SCL(0);							//拉低CST816D_I2C_SCL，开始下一个时序模块
}

/**
  * 函    数：I2C接收应答位
  * 参    数：无
  * 返 回 值：接收到的应答位，范围：0~1，0表示应答，1表示非应答
  */
uint8_t CST816D_I2C_ReceiveAck(void)
{
	uint8_t AckBit;							//定义应答位变量
	CST816D_I2C_SDA(1);							//接收前，主机先确保释放CST816D_I2C_SDA，避免干扰从机的数据发送
	CST816D_I2C_SCL(1);							//释放CST816D_I2C_SCL，主机机在CST816D_I2C_SCL高电平期间读取CST816D_I2C_SDA
	AckBit = R_CST816D_I2C_SDA();					//将应答位存储到变量里
	CST816D_I2C_SCL(0);							//拉低CST816D_I2C_SCL，开始下一个时序模块
	return AckBit;							//返回定义应答位变量
}


void CST816D_I2C_WriteReg(uint8_t Reg, uint8_t Data)
{
	CST816D_I2C_Start();						//I2C起始
	CST816D_I2C_SendByte(CST816D_Address & 0xFE);		    //发送从机地址，写模式
	CST816D_I2C_ReceiveAck();					//接收应答
	CST816D_I2C_SendByte(Reg);				//发送寄存器地址
	CST816D_I2C_ReceiveAck();					//接收应答
	CST816D_I2C_SendByte(Data);				//发送数据
	CST816D_I2C_ReceiveAck();					//接收应答
	CST816D_I2C_Stop();						//I2C终止
}	

// 通用 I2C 读寄存器函数（所有传感器都能用）
uint8_t CST816D_I2C_ReadReg(uint8_t Reg)
{
    uint8_t Data;
    CST816D_I2C_Start();
    CST816D_I2C_SendByte(CST816D_Address & 0xFE);      // 写地址
    CST816D_I2C_ReceiveAck();
    CST816D_I2C_SendByte(Reg);            // 寄存器
    CST816D_I2C_ReceiveAck();
    
    CST816D_I2C_Start();                  // 重复起始
    CST816D_I2C_SendByte(CST816D_Address | 0x01);  // 读地址
    CST816D_I2C_ReceiveAck();
    
    Data = CST816D_I2C_ReceiveByte();      // 读数据
    CST816D_I2C_SendAck(1);               // 主机发送 NACK
    CST816D_I2C_Stop();
    
    return Data;
}

void CST816D_I2C_WriteArray(uint8_t Reg, uint8_t* Data, uint16_t Len)
{
	CST816D_I2C_Start();
	CST816D_I2C_SendByte(CST816D_Address);      // 写地址
	CST816D_I2C_ReceiveAck();
	CST816D_I2C_SendByte(Reg);            // 寄存器
	CST816D_I2C_ReceiveAck();
	
	for(uint16_t i=0; i<Len; i++)
	{
		CST816D_I2C_SendByte(Data[i]);     // 发送数据数组
		CST816D_I2C_ReceiveAck();
	}

	CST816D_I2C_Stop();
}

void CST816D_I2C_ReadArray(uint8_t Reg, uint8_t* Data, uint16_t Len)
{
	
	CST816D_I2C_Start();
	CST816D_I2C_SendByte(CST816D_Address);      // 写地址
	CST816D_I2C_ReceiveAck();
	CST816D_I2C_SendByte(Reg);            // 寄存器
	CST816D_I2C_ReceiveAck();
	
	CST816D_I2C_Start();                  // 重复起始
	CST816D_I2C_SendByte(CST816D_Address | 0x01);  // 读地址
	CST816D_I2C_ReceiveAck();
	
	for(uint16_t i=0; i<Len; i++)
	{
		Data[i] = CST816D_I2C_ReceiveByte();  // 接收数据到数组
		if(i < Len - 1)
			CST816D_I2C_SendAck(0);           // 主机发送 ACK，继续接收
		else
			CST816D_I2C_SendAck(1);           // 最后一个字节，发送 NACK
	}

	CST816D_I2C_Stop();
}
