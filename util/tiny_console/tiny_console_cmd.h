#ifndef __TINY_CONSOLE_CMD_H__
#define __TINY_CONSOLE_CMD_H__

#include "tiny_console.h"
#include "util/gnu_attributes.h"

#define CONSOLE_CMD_DEF(name) \
    int name(console_t* console, const int argc, const char** argv)

#define CONSOLE_CMD_UNUSE_ARGS \
    (void) console;               \
    (void) argc;               \
    (void) argv

#define CONSOLE_CMD_SECTION         .console.cmd
#define CONSOLE_BUILTIN_CMD_SECTION .console.builtin.cmd

#define EXPORT_CONSOLE_CMD_IN_SEC(sec, _cmd, _fn, _desc)  \
    GNU_SECTION(sec)                                      \
    const console_t::cmd_desc_t __console_cmd_desc_##_fn = { \
        .cmd = _cmd,                                      \
        .desc = _desc,                                    \
        .fn = _fn,                                        \
    }

#define EXPORT_CONSOLE_CMD(cmd, fn, desc) \
    EXPORT_CONSOLE_CMD_IN_SEC(CONSOLE_CMD_SECTION, cmd, fn, desc)

#if 1 == CONFIG_CONSOLE_BUILTIN_CMD_ENABLE
#define EXPORT_CONSOLE_BUILTIN_CMD(cmd, fn, desc) \
    EXPORT_CONSOLE_CMD_IN_SEC(CONSOLE_BUILTIN_CMD_SECTION, cmd, fn, desc)
#else
#define EXPORT_CONSOLE_BUILTIN_CMD(cmd, fn, desc) \
    GNU_UNUSED console_t::cmd_desc_t* __ptr_to_##fn = NULL
#endif

#endif // __TINY_CONSOLE_CMD_H__
