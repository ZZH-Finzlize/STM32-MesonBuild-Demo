#include <stdarg.h>
#include <stdio.h>
#include "prints.h"

void print_str(USART_TypeDef* usartx, const char* str)
{
    while ('\0' != *str) print_char(usartx, *str++);
}

void print_hex(USART_TypeDef* usartx, uint32_t num)
{
    for (int i = 0; i < 8; i++) {
        uint8_t val = (num & 0xF0000000) >> 28;

        if (val > 9)
            print_char(usartx, (val - 10) + 'A');
        else
            print_char(usartx, val + '0');

        num <<= 4;
    }
}

void print_dec(USART_TypeDef* usartx, uint32_t num)
{
    uint32_t power = 1000000000;
    uint8_t num_flag = 0;

    for (int i = 0; i < 10; i++) {
        uint8_t val = (num / power) % 10;

        if (0 != val)
            num_flag = 1;

        if (0 != num_flag)
            print_char(usartx, val + '0');

        power /= 10;
    }
}

// void print_double(USART_TypeDef* usartx, double num)
// {
// }

int usart_printf(USART_TypeDef* usartx, const char* fmt, ...)
{
    va_list vargs;
    char printf_buf[128];
    va_start(vargs, fmt);

    int len = vsnprintf(printf_buf, sizeof(printf_buf), fmt, vargs);

    for (int i = 0; i < len; i++) print_char(usartx, printf_buf[i]);

    va_end(vargs);
    return len;
}
