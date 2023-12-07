#include "mem_mana.h"
#include "mem_conf.h"
#include "mem_pools.h"
#include "util/iterators.h"
#include "util/container_of.h"
#include "util/linker_tools.h"

static uint32_t __mem0__[CONFIG_DEFAULT_POOL_SIZE * 1024 / 4];

#if CONFIG_ENABLE_TEST_CASES == 1
static uint32_t __mem_test_case__[CONFIG_TESTCASE_POOL_SIZE * 1024 / 4];
#endif

MemPool_t __MemPools__[] = {
    PoolByArray(MEMPOOL_DEFAULT, __mem0__),
#if CONFIG_ENABLE_TEST_CASES == 1
    PoolByArray(MEMPOOL_TESTCASE, __mem_test_case__),
#endif
};

static uint32_t allMemPoolSize = 0;

void memInit(void)
{
    ITER_ARRAY (pMember, __MemPools__) { // initial all mem pool
        pMemBlock_t pMemHeader = pMember->memStart;
        pMember->availableSize =
            (size_t) pMember->memEnd - (size_t) pMember->memStart;
        pMemHeader->isFree = true;
        pMemHeader->prev = nullptr;
        pMemHeader->next = nullptr;
        pMemHeader->size = pMember->availableSize;
        allMemPoolSize += pMember->availableSize;
    }
}

EXPORT_INIT_FUNC(memInit, 9);

void* memAlloc(size_t size, const uint32_t pool)
{
    if (size == 0)
        return NULL;

    if (pool >= ARRAY_SIZE(__MemPools__))
        return NULL;

    pMemBlock_t pMemHeader = __MemPools__[pool].memStart;
    // byte align
    size = (size + 0x0000003) & ~0x00000003;

    // minminum size to allocate a block
    size += sizeof(MemBlock_t);
    size_t sizeReq = size + sizeof(MemBlock_t);

    while (pMemHeader != NULL) {
        // must greater than, otherwise the next block dosen't have memory
        if (pMemHeader->isFree && pMemHeader->size > sizeReq) {
            // setup a new block after the allocated block
            pMemBlock_t newHeader = (pMemBlock_t) ((char*) pMemHeader + size);
            newHeader->isFree = true;
            newHeader->size = pMemHeader->size - sizeReq;
            newHeader->prev = pMemHeader;
            newHeader->next = NULL;

            pMemHeader->next = newHeader;
            pMemHeader->size = sizeReq;
            pMemHeader->isFree = false;

            __MemPools__[pool].availableSize -= sizeReq;

            return (char*) pMemHeader->mem;
        } else {
            pMemHeader = pMemHeader->next;
        }
    }
    return NULL;
}

void memFree(void* pMem)
{
    if (NULL == pMem)
        return;

    pMemBlock_t block = container_of(pMem, MemBlock_t, mem);

    if (block == NULL || block->isFree)
        return;

    pMemPool_t pool = NULL;

    ITER_ARRAY (pMember, __MemPools__) {
        if (pMem < (void*) pMember->memEnd
            && pMem > (void*) pMember->memStart) {
            pool = pMember;
            break;
        }
    }

    if (pool == NULL)
        return;

    pool->availableSize += block->size;

    block->isFree = true;

    pMemBlock_t prev = block->prev, next = block->next;

    // check the next block for combine
    if (next && next->isFree) {
        block->size += next->size;
        block->next = next->next;
        if (next->next)
            next->next->prev = block;
    }

    // check the prev block for combine
    if (prev && prev->isFree) {
        prev->size += block->size;
        prev->next = block->next;
        if (block->next)
            block->next->prev = prev;
    }
}

bool memIsClean(const uint32_t pool)
{
    MemPool_t* pMember = &__MemPools__[pool];
    uint32_t currentAvailableSize = 0;

    currentAvailableSize =
        (size_t) pMember->memEnd - (size_t) pMember->memStart;

    return currentAvailableSize == pMember->availableSize;
}

bool memIsCleanAll(void)
{
    FOR_ARRAY_I (__MemPools__) {
        if (false == memIsClean(i))
            return false;
    }

    return true;
}
