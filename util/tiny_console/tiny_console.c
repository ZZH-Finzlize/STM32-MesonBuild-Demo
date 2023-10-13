#include <stdio.h>
#include <stdarg.h>
#include "tiny_console.h"
#include "util/mem_mana/mem_mana.h"
#include "util/value_ops.h"
#include "util/iterators.h"

int console_init(console_t* this, uint32_t buffer_size, console_out_t output_fn,
                 const char* prefix)
{
    CHECK_PTR(this, -EINVAL);
    RETURN_IF_NZERO(this->rxbuf, -EBUSY);
    RETURN_IF_NZERO(this->txbuf, -EBUSY);
    RETURN_IF_NZERO(this->command_table, -EBUSY);

    this->command_table = map_create(31, bkdr_hash);
    CHECK_PTR(this->command_table, -ENOMEM);

    this->rxbuf = memAlloc(buffer_size, CONSOLE_MEM_POOL);
    CHECK_PTR_GOTO(this->rxbuf, rxbuf_err);

    this->txbuf = memAlloc(buffer_size, CONSOLE_MEM_POOL);
    CHECK_PTR_GOTO(this->txbuf, txbuf_err);

    this->buffer_size = buffer_size;
    this->prefix = prefix;
    this->cwd = "~";  // for now, we don't support filesystem
    this->write = output_fn;
    this->rx_idx = 0;
    this->tx_idx = 0;
    this->last_rx_idx = 0;

    return 0;

rxbuf_err:
    map_delete(this->command_table);
txbuf_err:
    memFree(this->rxbuf);
    return -ENOMEM;
}

console_t* console_create(uint32_t buffer_size, console_out_t output_fn,
                          const char* prefix)
{
    console_t* this = memAlloc(sizeof(console_t), CONSOLE_MEM_POOL);

    CHECK_PTR(this, NULL);

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
        } else {  // buffer is not full, then copy the data into buffer
            uint32_t buffer_free_space = this->buffer_size - this->tx_idx;
            uint32_t copy_len = MIN(buffer_free_space, len);

            memcpy(&this->txbuf[this->tx_idx], str, copy_len);
            this->tx_idx += copy_len;
            len -= copy_len;
        }
    }

    return 0;
}

static uint32_t parse_arg_num(const char* str)
{
    CHECK_PTR(str, 0);

    uint32_t arg_num = 0;

    while (*str++) {
        if (' ' == *str)
            arg_num++;
    }

    return arg_num;
}

static int console_execute(console_t* this)
{
    char** arg_arr = NULL;
    uint32_t arg_num = 0;

    char* first_arg = strchr(this->rxbuf, ' ');

    if (NULL != first_arg) {
        arg_num = parse_arg_num(first_arg);
        arg_arr = memAlloc(sizeof(char*) * arg_num, CONSOLE_MEM_POOL);
        CHECK_PTR(arg_arr, -ENOMEM);

        char* cur_arg = first_arg;
        uint32_t arg_idx = 0;

        while ('\0' != *cur_arg) {
            if (' ' == *cur_arg) {
                *cur_arg = '\0';
                arg_arr[arg_idx] = cur_arg;
                arg_idx++;

                if (arg_idx == arg_num)
                    break;
            }

            cur_arg++;
        }
    }

    console_cmdfn_t cmd_cb = NULL;
    int cmd_res = 0;
    int search_res =
        map_search(this->command_table, this->rxbuf, (map_value_t*)&cmd_cb);
    RETURN_IF(search_res < 0, -ENODEV);

    const char* chars = "\n";
    this->write(this, chars, 1);

    cmd_res = cmd_cb(this, arg_num, (const char**)arg_arr);

    if (NULL != arg_arr)
        memFree(arg_arr);

    return cmd_res;
}

void console_update(console_t* this)
{
    CHECK_PTR(this, );

    // should be the recived len
    uint32_t recived_len = this->rx_idx - this->last_rx_idx;
    FOR_I(recived_len)
    {
        char ch = this->rxbuf[this->rx_idx + i - 1];
        switch (ch) {
            case '\b':
            case '\177':
                if (this->rx_idx <= 0)
                    break;

                console_send_char(this, '\b');
                this->rx_idx -= 2;

                if ('\177' == ch) {
                    console_send_str(this, "\033[J");
                    this->rxbuf[this->rx_idx] = '\0';
                }
                break;

            case '\n':
                this->rxbuf[this->rx_idx - 1] = '\0';

                if (this->rx_idx >= 1) {
                    console_send_char(this, '\n');
                    this->last_ret_v = console_execute(this);
                    if (this->last_ret_v == -ENODEV) {
                        // this->rxbuf[this->rx_idx - 1] = '\0';
                        console_send_str(this, this->rxbuf);
                        console_send_str(this, ": no such command");
                    }
                }

                this->rx_idx = 0;
                this->rxbuf[0] = '\0';
                console_send_char(this, '\n');
                console_display_prefix(this);
                break;

            /* ignore list */
            case '\r':
            case '\t':
            case '\026':
                this->rx_idx--;
                break;

            default:
                if (this->rx_idx >= this->buffer_size) {
                    console_send_str(
                        this, "console buffer full, drop previous data\r\n");
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

    this->last_rx_idx = this->rx_idx;
    console_flush(this);
}
