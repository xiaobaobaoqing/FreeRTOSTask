#include "Bluetooth.h"

/* ============ 对外全局 ============ */
char    Serial_RxPacket[100];
uint8_t Serial_RxFlag = 0;

/* ============ HAL 句柄 ============ */
UART_HandleTypeDef huart1;

/* 单字节接收缓冲，HAL_UART_Receive_IT 每收 1 字节进一次回调 */
static uint8_t s_rx_byte;

/* 兼容旧标准库的 USART1 指针 */
#define USART1_INSTANCE   USART1

/* =========================================================
 * 初始化
 * ========================================================= */
void Serial_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. 时钟 */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. PA9 = USART1_TX, PA10 = USART1_RX, AF7 */
    GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 3. USART1 参数（和标准库一致） */
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 4. NVIC
       ★ FreeRTOS 环境下：优先级必须 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY（通常是 5）
       否则在 ISR 里调用 FreeRTOS API 会直接 assert 死机 */
    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    /* 5. 挂上单字节接收中断，HAL 会自动开 RXNE */
    HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
}

/* =========================================================
 * 发送
 * ========================================================= */
void Serial_SendByte(uint8_t Byte)
{
    HAL_UART_Transmit(&huart1, &Byte, 1, HAL_MAX_DELAY);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    HAL_UART_Transmit(&huart1, Array, Length, HAL_MAX_DELAY);
}

void Serial_SendString(char *String)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)String, strlen(String), HAL_MAX_DELAY);
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) Result *= X;
    return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/* printf 重定向：Keil MDK 用 fputc，GCC 用 _write */
#ifdef __GNUC__
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}
#else
int fputc(int ch, FILE *f)
{
    Serial_SendByte((uint8_t)ch);
    return ch;
}
#endif

void Serial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    Serial_SendString(String);
}

/* =========================================================
 * ★ 关键：接收完成回调（替代标准库里的 USART1_IRQHandler 状态机）
 *   HAL 每收到 1 个字节会自动进这里一次
 * ========================================================= */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
    if (huart->Instance == USART1)
    {

        static uint8_t RxState    = 0;
        static uint8_t pRxPacket  = 0;
        uint8_t RxData = s_rx_byte;

        if (RxState == 0)
        {
            if (RxData == '@' && Serial_RxFlag == 0)
            {
                RxState   = 1;
                pRxPacket = 0;
            }
        }
        else if (RxState == 1)
        {
            if (RxData == '\r')
            {
                RxState = 2;
            }
            else
            {
                /* 防止越界 */
                if (pRxPacket < sizeof(Serial_RxPacket) - 1)
                {
                    Serial_RxPacket[pRxPacket++] = RxData;
                }
            }
        }
        else if (RxState == 2)
        {
            if (RxData == '\n')
            {
                RxState = 0;
                Serial_RxPacket[pRxPacket] = '\0';
                Serial_RxFlag = 1;
            }
        }

        /* ★ 重新挂上下一次接收（HAL 必须在回调里重挂，否则只收一次） */
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
    }
}

/* 错误回调（可选）：溢出/帧错 时重挂 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);
    }
}

/* =========================================================
 * ★ USART1_IRQHandler 由 HAL 自动处理
 *   stm32f4xx_it.c 里只保留一句调用即可：
 *
 *   void USART1_IRQHandler(void)
 *   {
 *       HAL_UART_IRQHandler(&huart1);
 *   }
 * ========================================================= */


 void Bluetooth_Init(void)
 {

	Serial_Init();

	GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_8;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;   /* 推挽输出 */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 上电默认关闭蓝牙模块（PA8 拉低） */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

	
 }

 void Bluetooth_ON(void)
 {
	__HAL_UART_ENABLE(&huart1);
    HAL_UART_Receive_IT(&huart1, &s_rx_byte, 1);   /* 重挂单字节接收 */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
 }
 void Bluetooth_OFF(void)
{
    HAL_UART_AbortReceive(&huart1);       /* 会终止接收，State → READY */
    __HAL_UART_DISABLE(&huart1);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}