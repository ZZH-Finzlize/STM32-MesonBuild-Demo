#include "builtin_cmds.h"

console_t* cur_con = NULL;

CONSOLE_CMD_DEF(help)
{
    CONSOLE_CMD_UNUSE_ARGS;

    cur_con = console;

    console->get_command_table()->foreach (
        [](map_t::key_t key, map_t::value_t value) {
            CHECK_PTR(cur_con, );
            (void) key;

            console_t::cmd_desc_t* desc = (console_t::cmd_desc_t*) value;

            cur_con->send_str(desc->cmd);
            cur_con->send_str(" - ");
            cur_con->send_str(desc->desc);
            cur_con->send_str("\r\n\r\n");
        });

    return 0;
}

EXPORT_CONSOLE_BUILTIN_CMD("help", help, "Get all command usages");
