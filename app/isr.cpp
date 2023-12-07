#include "stm32f10x.h"
#include "util/tiny_console/tiny_console.h"

#ifdef __cplusplus
extern "C" {
#endif

extern console_t* console;
extern volatile uint8_t rcv_flag;

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        console_input_char(console, USART_ReceiveData(USART1));
        rcv_flag = 1;
    }

    // asm volatile('')
}

#ifdef __cplusplus
}
#endif
