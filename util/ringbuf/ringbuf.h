#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <cstddef>
#include <cstdint>
#include "util/arg_checkers.h"

class ringbuf_t {
    uint32_t _wpos;
    uint32_t _rpos;
    uint32_t _size;
    uint8_t* buf;

    class ringbuf_locked_t {
        ringbuf_t* prb;
        uint8_t lock;
    };

    /**
     *@brief create a ringbuf
     *
     * @param buffer - buffer address
     * @param size - buffer size
     */
    ringbuf_t(uint32_t buffer_size)
    {
        RETURN_IF_ZERO(buffer_size, );

        this->buf = new uint8_t[buffer_size];

        CHECK_PTR(this->buf, );

        this->_size = buffer_size;
        this->_rpos = 0;
        this->_wpos = 0;
    }

    /**
     * @brief delete a ringbuf
     *
     */
    ~ringbuf_t()
    {
        delete[] this->buf;
    }

    /**
     * @brief clear a ring buf
     *
     * @param this - ringbuf struct pointer
     */
    inline void clear(void)
    {
        this->_rpos = this->_wpos = 0;
    }

    /**
     *@brief check the given ringbuf is empty or not
     *
     * @return bool false: not empty, true: empty
     */
    inline bool empty(void) const
    {
        return this->_wpos == this->_rpos;
    }

    /**
     * @brief check the given rinbuf is full or not
     *
     * @return uint8_t false: not full, true: full
     */
    inline bool full(void) const
    {
        if (0 == this->_rpos)
            return this->_wpos == this->_size - 1;
        else
            return this->_wpos == this->_rpos - 1;
    }

    /**
     *@brief get ringbuf size
     *
     * @return uint32_t - size of ringbuf
     */
    inline uint32_t size(void) const
    {
        return this->_size;
    }

    /**
     * @brief get writeable address
     *
     * @return void* writeable pointer
     */
    inline void* wpos(void)
    {
        return &this->buf[this->_wpos];
    }

    /**
     * @brief get readable address
     *
     * @return void* readable pointer
     */
    inline void* rpos(void)
    {
        return &this->buf[this->_rpos];
    }

    /**
     * @brief get the length of continuous memory that could be read
     *
     * @return uint32_t length of the continuous memory
     */
    inline uint32_t get_seq_read_len(void) const
    {
        return this->_size - this->_rpos;
    }

    /**
     * @brief get the length of continuous memory that could be write
     *
     * @return uint32_t length of the continuous memory
     */
    inline uint32_t get_seq_write_len(void) const
    {
        if (this->_wpos >= this->_rpos)
            return this->_size - this->_wpos;
        else
            return this->_rpos - this->_wpos;
    }

    // w > r
    // 0 1 2  3 4 5 6 7 8  9 10 11
    //[][][r][][][][][][w][][][]
    // free bytes = 12-8 = 4 + 2 - 1 = 5(size-w+r-1)
    // available bytes = 8-2 = 6 = (w-r)

    // r > w
    // 0 1 2  3 4 5 6 7 8  9 10 11
    //[][][w][][][][][][r][][][]
    // free bytes = 8-2-1 = 5 (r-w-1)
    // available bytes = 12-8 = 4 + 2 = 6(size-r+w)

    // w > r
    // 0 1 2  3 4 5 6  7 8 9 10 11
    //[][][r][][][][w][][][][][]
    // free bytes = 12-6 = 6 + 2 - 1 = 7(size-w+r-1)
    // available bytes = 6-2 = 4 = (w-r)

    // r > w
    // 0 1 2  3 4 5 6  7 8 9 10 11
    //[][][w][][][][r][][][][][]
    // free bytes = 6-2-1 = 3 (r-w-1)
    // available bytes = 12-6 = 6 + 2 = 8(size-r+w)

    /**
     *@brief get free space of a ringbuf (how many bytes could be write)
     *
     * @return uint32_t free bytes
     */
    inline uint32_t get_free_bytes(void) const
    {
        if (this->_wpos >= this->_rpos)
            return this->_size - this->_wpos + this->_rpos - 1;
        else
            return this->_rpos - this->_wpos - 1;
    }

    /**
     *@brief get available bytes (how many bytes could be read)
     *
     * @return uint32_t avaliable bytes
     */
    inline uint32_t get_available_bytes(void) const
    {
        if (this->_wpos >= this->_rpos)
            return this->_wpos - this->_rpos;
        else
            return this->_size - this->_rpos + this->_wpos;
    }

    /**
     *@brief increase internal write pos (may be write by dma)
     *
     * @param num - how many bytes needs to be increase
     */
    inline void increase_bytes(uint32_t num)
    {
        this->_wpos += num;
        this->_wpos %= this->_size;
    }

    /**
     *@brief increase internal read pos (drop some bytes, may be read by dma)
     *
     * @param num how many bytes needs to be drop
     */
    inline void drop_bytes(uint32_t num)
    {
        this->_rpos += num;
        this->_rpos %= this->_size;
    }

    /**
     *@brief get internal buffer pointer
     *
     * @return void* - buffer pointer
     */
    inline void* get_buffer(void)
    {
        return this->buf;
    }

    /**
     *@brief write data to ringbuf
     *
     * @param data - data address
     * @param size - data length
     * @return uint32_t - actual write length
     */
    uint32_t write(void* data, uint32_t size);

    /**
     *@brief read data to ringbuf
     *
     * @param data - data address
     * @param size - data length
     * @return uint32_t - actual read length
     */
    uint32_t read(void* data, uint32_t size);

    /**
     *@brief write one byte to ringbuf
     *
     * @param byte - byte data
     */
    void write_byte(uint8_t byte);

    /**
     *@brief read one byte from ringbuf
     *
     * @return uint8_t - byte data
     */
    uint8_t read_byte(void);

    /**
     *@brief find a specific byte data in ringbuf
     *
     * @param byte - byte data
     * @return void* - data pos
     */
    void* search_byte(uint8_t byte);
};

#endif // __RINGBUF_H__
