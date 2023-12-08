#include "ringbuf.h"
#include "util/value_ops.h"

uint32_t ringbuf_t::write(void* data, uint32_t size)
{
    uint32_t free_size = this->get_free_bytes();
    uint32_t actual_size = MIN(size, free_size);
    uint8_t* pdata = static_cast<uint8_t*>(data);

    for (uint32_t i = 0; i < actual_size; i++) {
        this->buf[this->_wpos] = *pdata;
        pdata++;
        this->increase_bytes(1);
    }

    return actual_size;
}

uint32_t ringbuf_t::read(void* data, uint32_t size)
{
    uint32_t available_bytes = this->get_available_bytes();
    uint32_t actual_size = MIN(size, available_bytes);
    uint8_t* pdata = static_cast<uint8_t*>(data);

    for (uint32_t i = 0; i < actual_size; i++) {
        *pdata = this->buf[this->_rpos];
        pdata++;
        this->drop_bytes(1);
    }

    return actual_size;
}

void ringbuf_t::write_byte(uint8_t byte)
{
    uint32_t free_size = this->get_free_bytes();

    if (free_size < 1)
        return;

    this->buf[this->_wpos] = byte;
    this->increase_bytes(1);
}

uint8_t ringbuf_t::read_byte()
{
    uint32_t available_bytes = this->get_available_bytes();

    if (available_bytes < 1)
        return 0;

    uint8_t byte = this->buf[this->_rpos];
    this->drop_bytes(1);

    return byte;
}

void* ringbuf_t::search_byte(uint8_t byte)
{
    uint32_t rpos = this->_rpos;
    uint32_t available_bytes = this->get_available_bytes();

    for (uint32_t i = 0; i < available_bytes; i++) {
        if (byte == this->buf[rpos])
            return &this->buf[rpos];
        rpos++;
        rpos %= this->_size;
    }

    return NULL;
}
