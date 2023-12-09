#ifndef __BIT_MAP_H__
#define __BIT_MAP_H__

#include <cstdint>
#include <cstring>
#include "util/iterators.h"
#include "util/arg_checkers.h"
#include "util/mem_mana/mem_mana.h"


#define BITMAP_CHECK_MAP(retv) CHECK_PTR(this->buf, retv)

class bitmap_t {
private:
    uint32_t len;
    uint32_t* buf;
    mem_pool_t pool;

public:
    /**
     * @brief create a bit map in the specific memory pool
     *
     * @param max_num - maximun value
     * @param pool - memory pool id
     */
    bitmap_t(uint32_t max_num, mem_pool_t pool);

    /**
     * @brief delete a bitmap
     *
     */
    ~bitmap_t()
    {
        delete[] this->buf;
    }

    inline uint32_t size(void) const
    {
        return this->len;
    }

    inline uint32_t length(void) const
    {
        return this->size();
    }

    /**
     * @brief clear a bitmap
     *
     */

    inline void clear(void)
    {
        BITMAP_CHECK_MAP();
        memset(this->buf, 0, this->len * sizeof(*this->buf));
    }

    /**
     * @brief save a value in given bitmap
     *
     * @param value - value to be saved
     */
    inline void save(uint32_t value)
    {
        BITMAP_CHECK_MAP();

        uint32_t unit_index = value / (sizeof(*this->buf) * 8);
        uint32_t bit_index = value % (sizeof(*this->buf) * 8);

        if (unit_index < this->len)
            this->buf[unit_index] |= 1 << bit_index;
    }

    /**
     * @brief delete a value from the given bitmap
     *
     * @param value - value to be deleted
     */
    inline void drop(uint32_t value)
    {
        BITMAP_CHECK_MAP();

        uint32_t unit_index = value / (sizeof(*this->buf) * 8);
        uint32_t bit_index = value % (sizeof(*this->buf) * 8);

        if (unit_index < this->len)
            this->buf[unit_index] &= ~(1 << bit_index);
    }

    /**
     * @brief check the given value is exist in the bitmap or not
     *
     * @param value - value to be check
     * @return bool true: exist, false: not exsit
     */
    inline bool check(uint32_t value)
    {
        BITMAP_CHECK_MAP(false);

        uint32_t unit_index = value / (sizeof(*this->buf) * 8);
        uint32_t bit_index = value % (sizeof(*this->buf) * 8);
        uint32_t bit_mask = 1 << bit_index;

        if (unit_index < this->len)
            return (this->buf[unit_index] & bit_mask) == bit_mask;

        return false;
    }

    /**
     * @brief find the first value in given bitmap
     *
     * @return uint32_t first freed value
     */
    uint32_t find_first_free(void);
};

#endif // __BIT_MAP_H__
