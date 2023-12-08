#include <stdlib.h>
#include "stm32f10x.h"
#include "test_frame.h"
#include "util/iterators.h"

#include "util/usart/prints.h"

LINKER_SYMBOL_TYPE(__stest_cases, __test_case_info_t);
LINKER_SYMBOL_TYPE(__etest_cases, __test_case_info_t);

LINKER_SYMBOL_TYPE(__sdemo, __demo_fn_t);
LINKER_SYMBOL_TYPE(__edemo, __demo_fn_t);

uint32_t get_all_testcases_num(void)
{
    return ((uint32_t) __etest_cases - (uint32_t) __stest_cases)
           / sizeof(*__stest_cases);
}

uint32_t run_all_testcases(void* arg)
{
#if CONFIG_ENABLE_TEST_CASES == 1
    uint32_t succ_count = 0;
    uint32_t test_case_num = get_all_testcases_num();

    FOR_I (test_case_num) {
        __test_case_info_t* test_case_info = &__stest_cases[i];
        usart_printf(USART1, "Running test case: %s\r\n", test_case_info->name);
        int retv = test_case_info->fn(arg);
        usart_printf(USART1, "Return value: %d\r\n", retv);
#if CONFIG_CHECK_TESTCASE_MEMPOOL == 1
        uint8_t isClean = memIsClean(CONFIG_TEST_CASE_MEMPOOLS);
        const char* fmt = isClean ? "clean" : "dirty";

        usart_printf(USART1, "Memory pool is %s!\r\n", fmt);
#endif
        usart_printf(USART1, "\r\n");
        succ_count += retv == 0;
    }

    return succ_count;
#else
    (void) arg;
    return 0;
#endif
}

void run_all_demo(void)
{
    uint32_t demo_num = ((uint32_t) __edemo - (uint32_t) __sdemo) / 4;

    FOR_I (demo_num) {
        __sdemo[i]();
    }
}
