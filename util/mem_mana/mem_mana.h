#ifndef __MEM_MANA_H__
#define __MEM_MANA_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mem_pools.h"

bool memIsClean(const mem_pool_t pool);
bool memIsCleanAll(void);

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, mem_pool_t pool);
void* operator new[](size_t size, mem_pool_t pool);

#endif // __MEM_MANA_H__
