#include <stdio.h>
#include <stdarg.h>
#include "tiny_console.h"
#include "tiny_console_cmd.h"

#include "util/value_ops.h"
#include "util/iterators.h"
#include "util/linker_tools.h"

void console_t::register_all_cmds(void)
{
    LINKER_SYMBOL_TYPE(__sconsole_cmd, cmd_desc_t);
    LINKER_SYMBOL_TYPE(__econsole_cmd, cmd_desc_t);

    cmd_desc_t* pcur_desc = __sconsole_cmd;

    while (pcur_desc < __econsole_cmd) {
        this->register_command(pcur_desc);
        pcur_desc++;
    }
}

console_t::console_t(uint32_t buffer_size, out_t output_fn, const char* prefix)
{
    RETURN_IF_NZERO(this->rxbuf, );
    RETURN_IF_NZERO(this->txbuf, );
    RETURN_IF_NZERO(this->command_table, );

    this->command_table = new map_t(31, bkdr_hash);
    CHECK_PTR(this->command_table, );

    this->rxbuf = new char[buffer_size];
    CHECK_PTR_GOTO(this->rxbuf, rxbuf_err);

    this->txbuf = new char[buffer_size];
    CHECK_PTR_GOTO(this->txbuf, txbuf_err);

    this->buffer_size = buffer_size;
    this->prefix = prefix;
    this->cwd = "~"; // for now, we don't support filesystem
    this->write = output_fn;
    this->rx_idx = 0;
    this->tx_idx = 0;
    this->last_rx_idx = 0;
    this->current_state = state_normal;

    this->register_all_cmds();

txbuf_err:
    delete this->rxbuf;
rxbuf_err:
    delete this->command_table;
    return;
}

int console_t::send_char(const char ch)
{
    // buffer full
    int retv = 0;

    if (this->tx_idx >= this->buffer_size) {
        retv = this->flush();
    }

    this->txbuf[this->tx_idx++] = ch;

    return retv;
}

int console_t::send_str(const char* str)
{
    CHECK_PTR(str, -EINVAL);

    uint32_t len = strlen(str);

    while (0 != len) {
        // buffer full
        if (this->tx_idx >= this->buffer_size) {
            // flush buffer && send all the data inside of it
            int retv = this->flush();
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

void console_t::display_prefix()
{
    this->set_color(green, black);
    this->send_str(this->prefix);
    this->cancel_color();

    this->send_char(':');

    this->set_color(blue, black);
    this->send_str(this->cwd);
    this->cancel_color();

    this->send_char('$');
    this->send_char(' ');
}

int console_t::printf(const char* fmt, ...)
{
    CHECK_PTR(fmt, -EINVAL);

    va_list vargs;
    va_start(vargs, fmt);

    uint32_t txbuf_free_size = this->buffer_size - this->tx_idx;
    int len =
        vsnprintf(&this->txbuf[this->tx_idx], txbuf_free_size, fmt, vargs);

    this->tx_idx += len;

    if (len == (int) txbuf_free_size)
        this->flush();

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

int console_t::execute(void)
{
    char** arg_arr = NULL;
    uint32_t arg_num = 0;

    char* first_arg = strchr(this->rxbuf, ' ');
    arg_num = parse_arg_num(first_arg);

    if (0 != arg_num) {
        arg_arr = new char*[arg_num];
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

    cmd_desc_t* cmd_desc = NULL;
    int cmd_res = 0;
    int search_res =
        this->command_table->search(this->rxbuf, (map_t::value_t*) &cmd_desc);
    RETURN_IF(search_res < 0, -ENODEV);

    this->send_strln("");

    cmd_res = cmd_desc->fn(this, arg_num, (const char**) arg_arr);

    if (NULL != arg_arr)
        delete[] arg_arr;

    return cmd_res;
}

void console_t::update_normal(char ch)
{
    switch (ch) {
        case '\b':
        case '\177':
            if (this->rx_idx < 2) {
                this->rx_idx--;
                break;
            }

            this->send_char('\b');
            this->rx_idx -= 2;

            if ('\177' == ch) {
                this->send_str("\033[J");
                this->rxbuf[this->rx_idx] = '\0';
            }
            break;

        case '\r':
            this->rxbuf[this->rx_idx - 1] = '\0';

            if (this->rx_idx > 1) {
                this->send_strln("");
                this->last_ret_v = this->execute();
                if (this->last_ret_v == -ENODEV) {
                    // this->rxbuf[this->rx_idx - 1] = '\0';
                    this->send_str(this->rxbuf);
                    this->send_str(": no such command");
                }
            }

            this->rx_idx = 0;
            this->rxbuf[0] = '\0';
            this->send_strln("");
            this->display_prefix();
            break;

        /* ignore list */
        case '\n':
        case '\t':
        case '\026': this->rx_idx--; break;

        // up arrow is \033A down \033B right \033C left \033D
        case '\033':
            this->rx_idx--;
            this->current_state = state_033;
            break;

        default:
            if (this->rx_idx >= this->buffer_size) {
                this->send_strln("console buffer full, drop previous data");
                this->rx_idx = 0;
                this->rxbuf[this->rx_idx] = '\0';
                this->display_prefix();
                this->flush();
                return;
            } else {
                this->send_char(ch);
            }
            break;
    }
}

void console_t::update_033(char ch)
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
        this->current_state = state_normal;

    this->rx_idx--;
}

void console_t::update(void)
{
    // should be the recived len
    uint32_t recived_len = this->rx_idx - this->last_rx_idx;
    RETURN_IF_ZERO(recived_len, );

    FOR_I (recived_len) {
        char ch = this->rxbuf[this->rx_idx + i - 1];

        switch (this->current_state) {
            case state_normal: this->update_normal(ch); break;

            case state_033: this->update_033(ch); break;

            default: break;
        }
    }

    this->last_rx_idx = this->rx_idx;
    this->flush();
}
