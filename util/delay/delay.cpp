#include "delay.h"

void NO_OPT delay_us_sw(uint32_t us);

void NO_OPT delay_ms_sw(uint32_t ms)
{
    for (size_t i = 0; i < 3000 * ms; i++) {
        asm volatile("nop");
    }
}

void NO_OPT delay_us_hw(uint32_t us);
void NO_OPT delay_ms_hw(uint32_t ms);
