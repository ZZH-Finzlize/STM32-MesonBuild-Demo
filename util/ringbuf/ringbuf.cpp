#include "ringbuf.h"
#include "util/value_ops.h"

uint32_t ringbuf_write(ringbuf_t* this, void* data, uint32_t size)
{
    RB_CHECK_POINTER(this, 0);

    uint32_t free_size = ringbuf_get_free_bytes(this);
    uint32_t actual_size = MIN(size, free_size);
    uint8_t* pdata = data;

    for (uint32_t i = 0; i < actual_size; i++) {
        this->buf[this->wpos] = *pdata;
        pdata++;
        ringbuf_increase_bytes(this, 1);
    }

    return actual_size;
}

uint32_t ringbuf_read(ringbuf_t* this, void* data, uint32_t size)
{
    RB_CHECK_POINTER(this, 0);

    uint32_t available_bytes = ringbuf_get_available_bytes(this);
    uint32_t actual_size = MIN(size, available_bytes);
    uint8_t* pdata = data;

    for (uint32_t i = 0; i < actual_size; i++) {
        *pdata = this->buf[this->rpos];
        pdata++;
        ringbuf_drop_bytes(this, 1);
    }

    return actual_size;
}

void ringbuf_write_byte(ringbuf_t* this, uint8_t byte)
{
    RB_CHECK_POINTER(this, );

    uint32_t free_size = ringbuf_get_free_bytes(this);

    if (free_size < 1)
        return;

    this->buf[this->wpos] = byte;
    ringbuf_increase_bytes(this, 1);
}

uint8_t ringbuf_read_byte(ringbuf_t* this)
{
    RB_CHECK_POINTER(this, 0);
    uint32_t available_bytes = ringbuf_get_available_bytes(this);

    if (available_bytes < 1)
        return 0;

    uint8_t byte = this->buf[this->rpos];
    ringbuf_drop_bytes(this, 1);

    return byte;
}

void* ringbuf_chb(ringbuf_t* this, uint8_t byte)
{
    RB_CHECK_POINTER(this, NULL);

    uint32_t rpos = this->rpos;
    uint32_t available_bytes = ringbuf_get_available_bytes(this);

    for (uint32_t i = 0; i < available_bytes; i++) {
        if (byte == this->buf[rpos])
            return &this->buf[rpos];
        rpos++;
        rpos %= this->size;
    }

    return NULL;
}
