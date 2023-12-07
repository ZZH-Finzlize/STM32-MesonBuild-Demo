#include "str_hash.h"

// BKDR Hash Function (recommended for string)
uint32_t bkdr_hash(const char *str)
{
    uint32_t seed = 131; // 31 131 1313 13131 131313 etc..
    uint32_t hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

uint32_t sdbm_hash(const char *str)
{
    uint32_t hash = 0;

    while (*str) {
        // equivalent to: hash = 65599*hash + (*str++);
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }

    return (hash & 0x7FFFFFFF);
}

// RS Hash Function
uint32_t rs_hash(const char *str)
{
    uint32_t b = 378551;
    uint32_t a = 63689;
    uint32_t hash = 0;

    while (*str) {
        hash = hash * a + (*str++);
        a *= b;
    }

    return (hash & 0x7FFFFFFF);
}

// JS Hash Function
uint32_t js_hash(const char *str)
{
    uint32_t hash = 1315423911;

    while (*str) {
        hash ^= ((hash << 5) + (*str++) + (hash >> 2));
    }

    return (hash & 0x7FFFFFFF);
}

// P. J. Weinberger Hash Function
uint32_t pjw_hash(const char *str)
{
    uint32_t BitsInUnignedInt = (uint32_t) (sizeof(uint32_t) * 8);
    uint32_t ThreeQuarters = (uint32_t) ((BitsInUnignedInt * 3) / 4);
    uint32_t OneEighth = (uint32_t) (BitsInUnignedInt / 8);
    uint32_t HighBits = (uint32_t) (0xFFFFFFFF)
                        << (BitsInUnignedInt - OneEighth);
    uint32_t hash = 0;
    uint32_t test = 0;

    while (*str) {
        hash = (hash << OneEighth) + (*str++);
        if ((test = hash & HighBits) != 0) {
            hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return (hash & 0x7FFFFFFF);
}

// ELF Hash Function
uint32_t elf_hash(const char *str)
{
    uint32_t hash = 0;
    uint32_t x = 0;

    while (*str) {
        hash = (hash << 4) + (*str++);
        if ((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }

    return (hash & 0x7FFFFFFF);
}

// DJB Hash Function
uint32_t djb_hash(const char *str)
{
    uint32_t hash = 5381;

    while (*str) {
        hash += (hash << 5) + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

// AP Hash Function
uint32_t ap_hash(const char *str)
{
    uint32_t hash = 0;
    int i;

    for (i = 0; *str; i++) {
        if ((i & 1) == 0) {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        } else {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }

    return (hash & 0x7FFFFFFF);
}
