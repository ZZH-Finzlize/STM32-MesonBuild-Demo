#include "stack_trace.h"
#include "util/linker_tools.h"
#include "util/usart/prints.h"

LINKER_SYMBOL32(__stack);

void print_stack_trace(USART_TypeDef *usartx, uint32_t *stack)
{
    for (uint32_t line = 0; line < STACK_TRACE_LENGTH; line++) {
        print_str(usartx, "0x");
        print_hex(usartx, (uint32_t) stack);
        print_char(usartx, ':');

        for (uint32_t i = 0; i < 6; i++) {
            if (stack >= __stack)
                return;

            print_char(usartx, ' ');
            print_hex(usartx, *stack++);
        }

        print_str(usartx, "\r\n");
    }
}
