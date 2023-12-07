#include "builtin_cmds.h"

console_t* cur_con = NULL;

static void iter_cb(map_key_t key, map_value_t value)
{
    CHECK_PTR(cur_con, );
    (void) key;

    console_cmd_desc_t* desc = (console_cmd_desc_t*) value;

    console_send_str(cur_con, desc->cmd);
    console_send_str(cur_con, " - ");
    console_send_str(cur_con, desc->desc);
    console_send_str(cur_con, "\r\n\r\n");
}

CONSOLE_CMD_DEF(help)
{
    CONSOLE_CMD_UNUSE_ARGS;

    cur_con = this;

    map_foreach(this->command_table, iter_cb);

    return 0;
}

EXPORT_CONSOLE_BUILTIN_CMD("help", help, "Get all command usages");
