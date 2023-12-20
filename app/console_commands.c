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

EXPORT_CONSOLE_CMD("run_tc", run_all_testcases_warp, "Run all testcases", NULL);

CONSOLE_CMD_DEF(run_all_demo_warp)
{
    CONSOLE_CMD_UNUSE_ARGS;

    run_all_demo();

    return 0;
}

EXPORT_CONSOLE_CMD("run_demo", run_all_demo_warp, "Run all demo", NULL);

CONSOLE_CMD_DEF(test)
{
    CONSOLE_CMD_UNUSE_ARGS;

    console_println(this, "argc: %d", argc);
    console_println(this, "argv[0]: %ld", (uint32_t) argv[0]);
    console_println(this, "argv[1]: %ld", (uint32_t) argv[1]);
    console_println(this, "argv[2]: %s", (char *) argv[2]);
    console_println(this, "argv[3]: %ld", (uint32_t) argv[3]);

    if (argc > 4) {
        console_println(this, "argv[4]: %ld", (uint32_t) argv[4]);
    }

    if (argc > 5) {
        console_println(this, "argv[5]: %s", (char *) argv[5]);
    }

    return 0;
}

EXPORT_CONSOLE_CMD("test", test, "test the console command", "ddsd[ds]");
