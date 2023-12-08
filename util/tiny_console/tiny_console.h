#ifndef __TINY_CONSOLE_H__
#define __TINY_CONSOLE_H__

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "tiny_console_conf.h"
#include "util/arg_checkers.h"
#include "util/gnu_attributes.h"

#include "util/map/map.h"

class console_t {
public:
    // return positive value when succ
    // return negative value when fail
    using out_t = int (*)(console_t*, const char*, uint32_t);

    typedef enum
    {
        black = '0',
        red,
        green,
        yellow,
        blue,
        purple,
        dark_green,
        white,
    } color_t;

    typedef enum
    {
        state_normal,
        state_033,
    } state_t;

private:
    const char *prefix, *cwd;
    uint32_t buffer_size;
    // uint32_t mem_pool;
    state_t current_state;
    int last_ret_v;

    out_t write;
    map_t* command_table;
    char *rxbuf, *txbuf;
    uint32_t rx_idx, tx_idx;
    uint32_t last_rx_idx;

    void register_all_cmds(void);
    int execute(void);
    void update_normal(char ch);
    void update_033(char ch);

public:
    console_t(uint32_t buffer_size, out_t output_fn, const char* prefix);

    ~console_t()
    {
        delete this->command_table;
        delete this->txbuf;
        delete this->rxbuf;
    }

    inline int flush(void)
    {
        RETURN_IF_ZERO(this->tx_idx, 0);

        uint32_t tx_idx = this->tx_idx;
        this->tx_idx = 0;
        return this->write(this, this->txbuf, tx_idx);
    }

    int send_str(const char* str);
    int send_char(const char ch);
    int printf(const char* fmt, ...) GNU_PRINTF(2, 3);

    int println(const char* fmt, ...) GNU_PRINTF(2, 3)
    {
    }

    inline int send_strln(const char* str)
    {
        int len = this->send_str(str);
        this->send_str("\r\n");
        return len + 2;
    }

    inline int set_color(color_t font_color, color_t back_color)
    {
        char buf[] = {"\033[4\0;3\0m"};
        buf[6] = font_color;
        buf[3] = back_color;
        return this->send_str(buf);
    }

    inline int cancel_color(void)
    {
        return this->send_str("\033[0m");
    }

    inline map_t* get_command_table(void)
    {
        return this->command_table;
    }

    void display_prefix(void);

    // return 0 when succ
    // return negative value when fail
    typedef int (*cmdfn_t)(console_t* console, const int argc,
                           const char** argv);

    typedef struct
    {
        const char* cmd;
        const char* desc;
        cmdfn_t fn;
    } cmd_desc_t;

    inline int register_command(cmd_desc_t* desc)
    {
        CHECK_PTR(desc, -EINVAL);

        return this->command_table->insert(desc->cmd, (size_t) desc);
    }

    inline int unregister_command(const char* cmd)
    {
        CHECK_PTR(cmd, -EINVAL);

        return this->command_table->remove(cmd);
    }

    void update(void);

    inline int input(char* data, uint32_t len)
    {
        CHECK_PTR(this->rxbuf, -EINVAL);
        RETURN_IF(this->rx_idx >= this->buffer_size, -ENOBUFS);

        for (uint32_t i = 0; i < len; i++) {
            this->rxbuf[this->rx_idx] = *data;
            this->rx_idx++;
            data++;
        }

        return 0;
    }

    inline int input_char(char data)
    {
        CHECK_PTR(this->rxbuf, -EINVAL);
        RETURN_IF(this->rx_idx >= this->buffer_size, -ENOBUFS);

        this->rxbuf[this->rx_idx] = data;
        this->rx_idx++;

        return 0;
    }
};

#endif // __TINY_CONSOLE_H__
