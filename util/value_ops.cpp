#include "value_ops.h"

GNU_PURE uint8_t ff1(size_t num)
{
    if (0 == num)
        return 0;

    uint8_t bitsOfVar = sizeof(num) * 8; // 通过sizeof实现自适应
    uint8_t index = 1;

    // 这里执行的时候变量的位数已经固定了, 所以实际上是O(1), 32位固定循环5次,
    // 64位固定循环6次
    while (bitsOfVar) {
        bitsOfVar /= 2;
        size_t tmp = num >> bitsOfVar;
        if (tmp) {
            index += bitsOfVar;
            num = tmp;
        }
    }

    return index;
}

GNU_PURE size_t reverseBits(size_t num, uint8_t bitsOfVar)
{
    if (0 == num || bitsOfVar > (sizeof(num) * 8))
        return 0;

    const size_t topBit = 1 << (bitsOfVar - 1);
    bitsOfVar >>= 1;

    for (uint8_t i = 0; i < bitsOfVar; i++) {
        size_t lowMask = 1 << i;
        size_t highMask = topBit >> i;
        uint8_t lowBit = (num & lowMask) > 0;
        uint8_t highBit = (num & highMask) > 0;

        num &= ~(highMask | lowMask);
        num |= (highMask * lowBit) | (lowMask * highBit);
    }

    return num;
}

uint32_t valueAt(uint32_t *pnum, uint32_t low_bit, uint32_t high_bit)
{
    if ((high_bit - low_bit > 32) || (NULL == pnum))
        return 0;

    // 0-31 32-63 64-95 96-127 ...
    uint32_t word_index = low_bit / 32;
    uint32_t word_boundry = (word_index + 1) * 32 - 1;
    uint8_t over_word_boundry = high_bit > word_boundry;
    uint32_t value = 0;

    low_bit %= 32;
    high_bit %= 32;

    if (over_word_boundry) { // separate in two words
        value = BITS_AT(pnum[word_index + 1], 0, high_bit) << (32 - low_bit);
        value |= BITS_AT(pnum[word_index], low_bit, 32);
    } else { // in a same word
        value = BITS_AT(pnum[word_index], low_bit, high_bit);
    }

    return value;
}
