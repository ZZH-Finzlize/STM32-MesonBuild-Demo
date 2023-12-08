#include "mem_mana.h"
#include "mem_conf.h"
#include "mem_pools.h"
#include "util/iterators.h"
#include "util/container_of.h"
#include "util/linker_tools.h"

typedef struct __MemBlock
{
    struct __MemBlock *next, *prev;
    uint32_t size;
    uint8_t isFree;
    // uint32_t mem[];
} MemBlock_t, *pMemBlock_t;

typedef struct
{
    MemBlock_t* const memStart;
    const void* const memEnd;
    uint32_t availableSize;
} MemPool_t, *pMemPool_t;

#define MemEnd(mem) (mem + ARRAY_SIZE(mem))
#define ArrayMem(arr)                     \
    {                                     \
        (pMemBlock_t) arr, MemEnd(arr), 0 \
    }

// void* memAlloc(size_t size, const uint32_t pool);
// void memFree(void* pMem);

static uint32_t __mem0__[CONFIG_DEFAULT_POOL_SIZE * 1024 / 4];

#if CONFIG_ENABLE_TEST_CASES == 1
static uint32_t __mem_test_case__[CONFIG_TESTCASE_POOL_SIZE * 1024 / 4];
#endif

MemPool_t __MemPools__[] = {
    ArrayMem(__mem0__),
#if CONFIG_ENABLE_TEST_CASES == 1
    ArrayMem(__mem_test_case__),
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

void* memAlloc(size_t size, const mem_pool_t pool)
{
    if (size == 0)
        return nullptr;

    if (pool >= ARRAY_SIZE(__MemPools__))
        return nullptr;

    pMemBlock_t pMemHeader = __MemPools__[pool].memStart;
    // byte align
    size = (size + 0x0000003) & ~0x00000003;

    // minminum size to allocate a block
    size += sizeof(MemBlock_t);
    size_t sizeReq = size + sizeof(MemBlock_t);

    while (pMemHeader != nullptr) {
        // must greater than, otherwise the next block dosen't have memory
        if (pMemHeader->isFree && pMemHeader->size > sizeReq) {
            // setup a new block after the allocated block
            pMemBlock_t newHeader = (pMemBlock_t) ((char*) pMemHeader + size);
            newHeader->isFree = true;
            newHeader->size = pMemHeader->size - sizeReq;
            newHeader->prev = pMemHeader;
            newHeader->next = nullptr;

            pMemHeader->next = newHeader;
            pMemHeader->size = sizeReq;
            pMemHeader->isFree = false;

            __MemPools__[pool].availableSize -= sizeReq;

            return reinterpret_cast<char*>(pMemHeader + 1);
        } else {
            pMemHeader = pMemHeader->next;
        }
    }
    return nullptr;
}

void memFree(void* pMem)
{
    if (nullptr == pMem)
        return;

    pMem = (void*) ((size_t) pMem - sizeof(MemBlock_t));
    pMemBlock_t block = static_cast<pMemBlock_t>(pMem);

    if (block == nullptr || block->isFree)
        return;

    pMemPool_t pool = nullptr;

    ITER_ARRAY (pMember, __MemPools__) {
        if (pMem < (void*) pMember->memEnd
            && pMem > (void*) pMember->memStart) {
            pool = pMember;
            break;
        }
    }

    if (pool == nullptr)
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

bool memIsClean(const mem_pool_t pool)
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
        if (false == memIsClean(static_cast<mem_pool_t>(i)))
            return false;
    }

    return true;
}

// void* operator new(size_t size)
// {
//     return memAlloc(size, mem_pool_t::default_pool);
// }

void* operator new(size_t size, mem_pool_t pool)
{
    return memAlloc(size, pool);
}

// void* operator new[](size_t size)
// {
//     return memAlloc(size, mem_pool_t::default_pool);
// }

void* operator new[](size_t size, mem_pool_t pool)
{
    return memAlloc(size, pool);
}

void operator delete(void* mem)
{
    memFree(mem);
}

void operator delete[](void* mem)
{
    memFree(mem);
}

void operator delete(void* mem, unsigned int t)
{
    (void) t;
    memFree(mem);
}

void operator delete[](void* mem, unsigned int t)
{
    (void) t;
    memFree(mem);
}
