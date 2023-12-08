#include "bitmap.h"
#include "util/value_ops.h"

bitmap_t::bitmap_t(uint32_t max_num)
{
    RETURN_IF_ZERO(max_num, );

    // uint32_t max_num_aligned = (max_num + 31) & ~31;
    uint32_t unit_num = max_num / (sizeof(*this->buf) * 8);
    this->buf = new uint32_t[unit_num];

    CHECK_PTR(this->buf, );

    this->len = unit_num;
}

uint32_t bitmap_t::find_first_free(void)
{
    BITMAP_CHECK_MAP(UINT32_MAX);

    const uint32_t bitmap_value_bits = sizeof(*this->buf) * 8;

    for (uint32_t i = 0; i < this->len; i++) {
        uint32_t value = this->buf[i];

        // ff0 algorithm - O(1) efficiency
        if (value < UINT32_MAX) // has free bits
        {
            uint32_t bits_of_var = bitmap_value_bits;
            uint32_t bit_index = 0;

            while (bits_of_var != 0 && value != 0) {
                bits_of_var /= 2;
                uint32_t bits_half_mask = FILL_BITS(bits_of_var);

                // lower bytes are full
                if ((value & bits_half_mask) == bits_half_mask) {
                    bit_index += bits_of_var;
                    value >>= bits_of_var; // take higher bytes
                }
            }

            return bit_index + i * bitmap_value_bits;
        }
    }

    return UINT32_MAX;
}
