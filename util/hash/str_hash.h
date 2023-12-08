#ifndef __STR_HASH_H__
#define __STR_HASH_H__

#include <cstdint>

using std::uint32_t;

typedef uint32_t (*str_hash_t)(const char *str);

uint32_t bkdr_hash(const char *str);
uint32_t sdbm_hash(const char *str);
uint32_t rs_hash(const char *str);
uint32_t js_hash(const char *str);
uint32_t pjw_hash(const char *str);
uint32_t elf_hash(const char *str);
uint32_t djb_hash(const char *str);
uint32_t ap_hash(const char *str);

#endif // __STR_HASH_H__
