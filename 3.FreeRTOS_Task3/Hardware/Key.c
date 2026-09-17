#include "Key.h"
#include "Bluetooth.h"



uint8_t Key(void)
{

    if (!Serial_RxFlag) return 0;


    char local[100];
    __disable_irq();
    strncpy(local, Serial_RxPacket, sizeof(local) - 1);
    local[sizeof(local) - 1] = '\0';
    Serial_RxFlag = 0;
    __enable_irq();

    /* 只接受单字符 '1'~'6' */
    if (local[0] >= '1' && local[0] <= '6' && local[1] == '\0')
    {
        return (uint8_t)(local[0] - '0');
    }
    return 0;
}


