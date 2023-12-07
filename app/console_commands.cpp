#include "test_frame.h"
#include "util/tiny_console/tiny_console_cmd.h"
#include "util/iterators.h"

CONSOLE_CMD_DEF(run_all_testcases_warp)
{
    CONSOLE_CMD_UNUSE_ARGS;

    uint32_t all_num = get_all_testcases_num();
    uint32_t succ_num = run_all_testcases(NULL);

    console_send_strln(this, "test result: ");
    console_println(this, "passed [%ld/%ld]", succ_num, all_num);
    console_println(this, "failed [%ld/%ld]", all_num - succ_num, all_num);

    return all_num == succ_num ? 0 : -EINVAL;
}

EXPORT_CONSOLE_CMD("run_tc", run_all_testcases_warp, "Run all testcases");

CONSOLE_CMD_DEF(run_all_demo_warp)
{
    CONSOLE_CMD_UNUSE_ARGS;

    console_println(this, "argc: %d", argc);

    FOR_I ((uint32_t) argc) {
        console_println(this, "arg[%ld]: %s", i, argv[i]);
    }

    run_all_demo();

    return 0;
}

EXPORT_CONSOLE_CMD("run_demo", run_all_demo_warp, "Run all demo");
