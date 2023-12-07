#include "bitmap.h"
#include "util/value_ops.h"

bitmap_t* bitmap_create_in_pool(uint32_t max_num, uint32_t pool)
{
    RETURN_IF_ZERO(max_num, NULL);

    uint32_t max_num_aligned = (max_num + 31) & ~31;
    bitmap_t* map =
        (bitmap_t*) memAlloc(sizeof(bitmap_t) + max_num_aligned / 8, pool);
    uint32_t unit_num = max_num / (sizeof(*map->buf) * 8);

    CHECK_PTR(map, NULL);

    map->len = unit_num;

    return map;
}

uint32_t bitmap_find_first_free(bitmap_t* map)
{
    BITMAP_CHECK_MAP(map, UINT32_MAX);

    const uint32_t bitmap_value_bits = sizeof(*map->buf) * 8;

    for (uint32_t i = 0; i < map->len; i++) {
        uint32_t value = map->buf[i];

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
