#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

#include "test_frame.h"

#include "util/delay/delay.h"
#include "util/iterators.h"
#include "util/tiny_console/tiny_console.h"

console_t* console = NULL;
volatile uint8_t rcv_flag = 0;

void clock_init(void)
{
    RCC_DeInit();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

void gpio_init(void)
{
    GPIO_InitTypeDef init_param;

    init_param.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);

    // 推挽输出
    init_param.GPIO_Mode = GPIO_Mode_Out_PP;

    // Led
    init_param.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &init_param);

    // 复用推挽
    init_param.GPIO_Mode = GPIO_Mode_AF_PP;

    // USART1 - TXD
    init_param.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOA, &init_param);

    // 浮空输入
    init_param.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    // USART1 - RXD
    init_param.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &init_param);
}

void usart_init(void)
{
    USART_InitTypeDef init_param = {
        .USART_BaudRate = 115200,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode = USART_Mode_Tx | USART_Mode_Rx,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    };

    USART_Init(USART1, &init_param);
    USART_Cmd(USART1, ENABLE);
}

void nvic_init()
{
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);

    NVIC_InitTypeDef init_param = {
        .NVIC_IRQChannel = USART1_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 12,
        .NVIC_IRQChannelSubPriority = 14,
        .NVIC_IRQChannelCmd = ENABLE,
    };

    NVIC_Init(&init_param);
    // USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

int console_output(console_t* console, const char* str, uint32_t len)
{
    (void) console;

    for (uint32_t i = 0; i < len; i++) {
        while (RESET == USART_GetFlagStatus(USART1, USART_FLAG_TC))
            ;
        USART_ClearFlag(USART1, USART_FLAG_TC);
        USART_SendData(USART1, str[i]);
    }
    return 0;
}

int main()
{
    clock_init();
    gpio_init();
    usart_init();
    nvic_init();

    // run_all_demo();
    // run_all_testcases(NULL);

    console = new console_t(64, console_output, "root@stm32");
    console->display_prefix();
    console->flush();

    while (1) {
        if (1 == rcv_flag) {
            rcv_flag = 0;
            console->update();
        }
    }

    return 0;
}
