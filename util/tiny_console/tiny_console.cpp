#include <stdio.h>
#include <stdarg.h>
#include "tiny_console.h"
#include "tiny_console_cmd.h"
#include "util/mem_mana/mem_mana.h"
#include "util/value_ops.h"
#include "util/iterators.h"
#include "util/linker_tools.h"

void console_register_all_cmds(console_t* this)
{
    LINKER_SYMBOL_TYPE(__sconsole_cmd, console_cmd_desc_t);
    LINKER_SYMBOL_TYPE(__econsole_cmd, console_cmd_desc_t);

    console_cmd_desc_t* pcur_desc = __sconsole_cmd;

    while (pcur_desc < __econsole_cmd) {
        console_register_command(this, pcur_desc);
        pcur_desc++;
    }
}

int console_init(console_t* this, uint32_t buffer_size, console_out_t output_fn,
                 const char* prefix)
{
    CHECK_PTR(this, -EINVAL);
    RETURN_IF_NZERO(this->rxbuf, -EBUSY);
    RETURN_IF_NZERO(this->txbuf, -EBUSY);
    RETURN_IF_NZERO(this->command_table, -EBUSY);

    this->command_table = map_create_in_pool(31, bkdr_hash, this->mem_pool);
    CHECK_PTR(this->command_table, -ENOMEM);

    this->rxbuf = memAlloc(buffer_size, this->mem_pool);
    CHECK_PTR_GOTO(this->rxbuf, rxbuf_err);

    this->txbuf = memAlloc(buffer_size, this->mem_pool);
    CHECK_PTR_GOTO(this->txbuf, txbuf_err);

    this->buffer_size = buffer_size;
    this->prefix = prefix;
    this->cwd = "~"; // for now, we don't support filesystem
    this->write = output_fn;
    this->rx_idx = 0;
    this->tx_idx = 0;
    this->last_rx_idx = 0;
    this->current_state = console_state_normal;

    console_register_all_cmds(this);

    return 0;

txbuf_err:
    memFree(this->rxbuf);
rxbuf_err:
    map_delete(this->command_table);
    return -ENOMEM;
}

console_t* console_create_in_pool(uint32_t buffer_size, console_out_t output_fn,
                                  const char* prefix, uint32_t pool)
{
    RETURN_IF(pool == UINT32_MAX, NULL);

    console_t* this = memAlloc(sizeof(console_t), pool);

    CHECK_PTR(this, NULL);

    this->mem_pool = pool;

    int retv = console_init(this, buffer_size, output_fn, prefix);

    if (0 != retv) {
        console_delete(this);
        return NULL;
    }

    return this;
}

int console_send_char(console_t* this, const char ch)
{
    CHECK_PTR(this, -EINVAL);
    // buffer full
    int retv = 0;

    if (this->tx_idx >= this->buffer_size) {
        retv = console_flush(this);
    }

    this->txbuf[this->tx_idx++] = ch;

    return retv;
}

int console_send_str(console_t* this, const char* str)
{
    CHECK_PTR(this, -EINVAL);
    CHECK_PTR(str, -EINVAL);

    uint32_t len = strlen(str);

    while (0 != len) {
        // buffer full
        if (this->tx_idx >= this->buffer_size) {
            // flush buffer && send all the data inside of it
            int retv = console_flush(this);
            RETURN_IF(retv < 0, retv);
        } else { // buffer is not full, then copy the data into buffer
            uint32_t buffer_free_space = this->buffer_size - this->tx_idx;
            uint32_t copy_len = MIN(buffer_free_space, len);

            memcpy(&this->txbuf[this->tx_idx], str, copy_len);
            this->tx_idx += copy_len;
            len -= copy_len;
            str += copy_len;
        }
    }

    return 0;
}

void console_display_prefix(console_t* this)
{
    console_set_color(this, concole_color_green, concole_color_black);
    console_send_str(this, this->prefix);
    console_cancel_color(this);

    console_send_char(this, ':');

    console_set_color(this, concole_color_blue, concole_color_black);
    console_send_str(this, this->cwd);
    console_cancel_color(this);

    console_send_char(this, '$');
    console_send_char(this, ' ');
}

int console_printf(console_t* this, const char* fmt, ...)
{
    CHECK_PTR(this, -EINVAL);
    CHECK_PTR(fmt, -EINVAL);

    va_list vargs;
    va_start(vargs, fmt);

    uint32_t txbuf_free_size = this->buffer_size - this->tx_idx;
    int len =
        vsnprintf(&this->txbuf[this->tx_idx], txbuf_free_size, fmt, vargs);

    this->tx_idx += len;

    if (len == (int) txbuf_free_size)
        console_flush(this);

    va_end(vargs);

    return len;
}

static uint32_t parse_arg_num(const char* str)
{
    CHECK_PTR(str, 0);

    uint32_t arg_num = 0;

    while (*str) {
        if ('\n' == *(str + 1) || '\0' == *(str + 1))
            return arg_num;
        else if (' ' == *str && ' ' != *(str + 1))
            arg_num++;

        str++;
    }

    return arg_num;
}

static int console_execute(console_t* this)
{
    char** arg_arr = NULL;
    uint32_t arg_num = 0;

    char* first_arg = strchr(this->rxbuf, ' ');
    arg_num = parse_arg_num(first_arg);

    if (0 != arg_num) {
        arg_arr = memAlloc(sizeof(char*) * arg_num, this->mem_pool);
        CHECK_PTR(arg_arr, -ENOMEM);

        char* cur_arg = first_arg;
        uint32_t arg_idx = 0;

        while ('\0' != *cur_arg) {
            if (' ' == *cur_arg) {
                *cur_arg = '\0';
                arg_arr[arg_idx] = cur_arg + 1;
                arg_idx++;

                if (arg_idx == arg_num)
                    break;
            }

            cur_arg++;
        }
    }

    if (NULL != first_arg)
        *first_arg = '\0';

    console_cmd_desc_t* cmd_desc = NULL;
    int cmd_res = 0;
    int search_res =
        map_search(this->command_table, this->rxbuf, (map_value_t*) &cmd_desc);
    RETURN_IF(search_res < 0, -ENODEV);

    console_send_strln(this, "");

    cmd_res = cmd_desc->fn(this, arg_num, (const char**) arg_arr);

    if (NULL != arg_arr)
        memFree(arg_arr);

    return cmd_res;
}

static inline void console_update_normal(console_t* this, char ch)
{
    switch (ch) {
        case '\b':
        case '\177':
            if (this->rx_idx < 2) {
                this->rx_idx--;
                break;
            }

            console_send_char(this, '\b');
            this->rx_idx -= 2;

            if ('\177' == ch) {
                console_send_str(this, "\033[J");
                this->rxbuf[this->rx_idx] = '\0';
            }
            break;

        case '\r':
            this->rxbuf[this->rx_idx - 1] = '\0';

            if (this->rx_idx > 1) {
                console_send_strln(this, "");
                this->last_ret_v = console_execute(this);
                if (this->last_ret_v == -ENODEV) {
                    // this->rxbuf[this->rx_idx - 1] = '\0';
                    console_send_str(this, this->rxbuf);
                    console_send_str(this, ": no such command");
                }
            }

            this->rx_idx = 0;
            this->rxbuf[0] = '\0';
            console_send_strln(this, "");
            console_display_prefix(this);
            break;

        /* ignore list */
        case '\n':
        case '\t':
        case '\026': this->rx_idx--; break;

        // up arrow is \033A down \033B right \033C left \033D
        case '\033':
            this->rx_idx--;
            this->current_state = console_state_033;
            break;

        default:
            if (this->rx_idx >= this->buffer_size) {
                console_send_strln(this,
                                   "console buffer full, drop previous data");
                this->rx_idx = 0;
                this->rxbuf[this->rx_idx] = '\0';
                console_display_prefix(this);
                console_flush(this);
                return;
            } else {
                console_send_char(this, ch);
            }
            break;
    }
}

static inline void console_update_033(console_t* this, char ch)
{
    bool exit_033 = true;

    switch (ch) {
        case 'A': // up arrow
            // console_send_strln(this, "up arrow");
            break;

        case 'B': // down arrow
            // console_send_strln(this, "down arrow");
            break;

        case 'C': // right arrow
            // console_send_strln(this, "right arrow");
            break;

        case 'D': // left arrow
            // console_send_strln(this, "left arrow");
            break;

        case '[': exit_033 = false; break;

        default: // ignore other
            break;
    }

    // back to normal state
    if (true == exit_033)
        this->current_state = console_state_normal;

    this->rx_idx--;
}

void console_update(console_t* this)
{
    CHECK_PTR(this, );

    // should be the recived len
    uint32_t recived_len = this->rx_idx - this->last_rx_idx;
    RETURN_IF_ZERO(recived_len, );

    FOR_I (recived_len) {
        char ch = this->rxbuf[this->rx_idx + i - 1];

        switch (this->current_state) {
            case console_state_normal: console_update_normal(this, ch); break;

            case console_state_033: console_update_033(this, ch); break;

            default: break;
        }
    }

    this->last_rx_idx = this->rx_idx;
    console_flush(this);
}
