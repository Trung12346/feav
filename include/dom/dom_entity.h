#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "debugger.h"

#define QWORD_SCAN_IS_OCCUPIED_MASK 0b0000000100000001000000010000000100000001000000010000000100000001ULL
#define DWORD_SCAN_IS_OCCUPIED_MASK 0b00000001000000010000000100000001UL
#define HALFWORD_SCAN_IS_OCCUPIED_MASK 0b00000001U
#define QWORD_WIDTH 64
#define DWORD_WIDTH 32
#define HALFWORD_WIDTH 8
#define POOL_STATUS_IS_OCCUPIED_BIT 0b00000001U
#define POOL_ELEMENT_COUNT 1024
#define POOL_STATUS_FLAGS_SIZE POOL_ELEMENT_COUNT * HALFWORD_WIDTH
#define ARENA_SIZE 1024 * 64
#define BIG_ARENA_SIZE 1024 * 1024 * 64

typedef uint64_t qword;
typedef uint32_t dword;
typedef uint16_t word;
typedef uint8_t hword;

typedef enum {
    CUSTOM,
    HTML,
    HEAD,
    BODY,
    DIV,
    META,
    H1,
    H2,
    H3,
    H4,
    H5,
    H6
} TagIdentifier;

typedef struct {
    uint16_t low;
    uint8_t high;
} uint24_t;

typedef struct {
    char tag_name[32];
    TagIdentifier tag_id;
    uintptr_t p_arena_attr;
    size_t arena_attr_size;
    uintptr_t p_arena_inner;
    size_t arena_inner_size;
    uintptr_t p_parent;
    uint8_t life_span_point;
} DOBJ;

typedef struct {
    uint16_t pool_index;
    uint16_t live_objects;
    uintptr_t p_prev;
    uintptr_t p_next;
    uint8_t status_flags[POOL_ELEMENT_COUNT];

    char tag_names[POOL_ELEMENT_COUNT][32];
    TagIdentifier tag_ids[POOL_ELEMENT_COUNT];
    uintptr_t p_arena_attrs[POOL_ELEMENT_COUNT];
    size_t arena_attr_sizes[POOL_ELEMENT_COUNT];
    uintptr_t p_arena_inner[POOL_ELEMENT_COUNT];
    size_t arena_inner_size[POOL_ELEMENT_COUNT];
    uintptr_t p_parent[POOL_ELEMENT_COUNT];
    uint8_t life_span_points[POOL_ELEMENT_COUNT];
} DOBJPool;

typedef struct {
    DOBJPool *pool;
    uint16_t item_index;
} DOBJHandler;

typedef struct {
    uint16_t arena_index;
    uint16_t *size_list;
    uint16_t size_list_size;
    uint16_t live_objects;
    uintptr_t p_prev;
    uintptr_t p_next;

    uint8_t mem[ARENA_SIZE]; //size: 64KB, page size: 1B
} Arena;

typedef struct {
    uint16_t arena_index;
    uint24_t *size_list;
    uint16_t size_list_size;
    uint16_t live_objects;
    uintptr_t p_prev;
    uintptr_t p_next;

    uint64_t mem[BIG_ARENA_SIZE / 8]; //size: 64MB, page size: 8B
} BigArena;

void pool_init(DOBJPool **pool, DOBJPool *prev_pool)
{
    *pool = malloc(sizeof(DOBJPool));

    (*pool)->live_objects = 0;

    if (prev_pool != NULL)
    {
        (*pool)->p_next = NULL;
        (*pool)->p_prev = prev_pool;
        prev_pool->p_next = *pool;
        (*pool)->pool_index = prev_pool->pool_index + 1;
    } else
    {
        (*pool)->p_next = NULL;
        (*pool)->p_prev = NULL;
        (*pool)->pool_index = 0;
    }
}

DOBJHandler pool_search(DOBJPool *pool, uint64_t pool_glb_index)
{
    DOBJPool *working_pool = pool;
    uint32_t pool_index = pool_glb_index / POOL_ELEMENT_COUNT;
    uint16_t item_index = pool_glb_index % POOL_ELEMENT_COUNT;

    uint32_t diff;
    if (pool->pool_index > pool_index)
    {
        diff = pool->pool_index - pool_index;
        for (uint32_t i = 0; i < diff; i++)
        {
            if (working_pool->p_prev == NULL)
            {
                printf(ERR SYS_DBG_PREFIX" Segmentation fault, invalid pool access");
                exit(1);
            }
            working_pool = working_pool->p_prev;
        }
    } else if (pool->pool_index < pool_index)
    {
        diff = pool_index - pool->pool_index;
        for (uint32_t i = 0; i < diff; i++)
        {
            if (working_pool->p_next == NULL)
            {
                printf(ERR SYS_DBG_PREFIX" Segmentation fault, invalid pool access");
                exit(1);
            }
            working_pool = working_pool->p_next;
        }
    }

    return (DOBJHandler){.pool = working_pool, .item_index = item_index};
}

uint16_t available_pool_region_search(DOBJPool *pool, bool *free_found)
{
    *free_found = false;
    for (uint32_t i = 0; i < POOL_STATUS_FLAGS_SIZE / QWORD_WIDTH; i++)
    {
        uint64_t buffer;
        uint32_t sbuffer;
        uint8_t ssbuffer;
        memcpy(&buffer, pool->status_flags + i * 8, 8);
        if (!(buffer & QWORD_SCAN_IS_OCCUPIED_MASK))
        {
            for (uint8_t si = 0; si < QWORD_WIDTH / DWORD_WIDTH; si++)
            {
                memcpy(&sbuffer, pool->status_flags + i * 8 + si * 4, 4);
                if (!(sbuffer & DWORD_SCAN_IS_OCCUPIED_MASK))
                {
                    for (uint8_t ssi = 0; ssi < DWORD_WIDTH / HALFWORD_WIDTH; ssi++)
                    {
                        memcpy(&ssbuffer, pool->status_flags + i * 8 + si * 4 + ssi, 1);
                        if (!(ssbuffer & HALFWORD_SCAN_IS_OCCUPIED_MASK))
                        {
                            *free_found = true;
                            return i * 8 + si * 4 + ssi;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
static uint64_t dobj_pool_alloc(DOBJ obj, DOBJPool *pool)
{
    bool free_found = false;
    DOBJPool *working_pool = pool;
    uint16_t working_index = available_pool_region_search(pool, &free_found);

    if (!free_found && pool->p_next == NULL)
    {
        pool_init(&working_pool, pool);
        working_index = 0;
    } else if (!free_found && pool->p_next != NULL)
    {
        return dobj_pool_alloc(obj, pool->p_next);
    }
    working_pool->live_objects++;
    working_pool->status_flags[working_index] |= POOL_STATUS_IS_OCCUPIED_BIT;
    strcpy(working_pool->tag_names[working_index], obj.tag_name);
    working_pool->tag_ids[working_index] = obj.tag_id;
    working_pool->p_arena_attrs[working_index] = obj.p_arena_attr;
    working_pool->arena_attr_sizes[working_index] = obj.arena_attr_size;
    working_pool->p_arena_inner[working_index] = obj.p_arena_inner;
    working_pool->arena_inner_size[working_index] = obj.arena_inner_size;
    working_pool->p_parent[working_index] = obj.p_parent;
    working_pool->life_span_points[working_index] = obj.life_span_point;

    return working_pool->pool_index * POOL_ELEMENT_COUNT + working_index;
}

void pool_free(DOBJPool *pool, uint64_t pool_glb_index)
{
    DOBJHandler obj = pool_search(pool, pool_glb_index);

    if (obj.pool->status_flags[obj.item_index] & POOL_STATUS_IS_OCCUPIED_BIT)
    {
        obj.pool->status_flags[obj.item_index] &= ~POOL_STATUS_IS_OCCUPIED_BIT;
        pool->live_objects--;
        if ((pool->live_objects < POOL_ELEMENT_COUNT / 2) && pool->p_next != NULL && ((DOBJPool *)pool->p_next)->live_objects == 0)
        {
            free((DOBJPool *)pool->p_next);
            pool->p_next = NULL;
        }
    }
}

//caching freed items in small/big cache.
//small cache doesnt run a definite algorithm, each freed index is placed in a particular pattern relative to the previously placed indices
//big cache needs a seperate CPU thread to occasionally run sorting algorithm, large overhead and slow, needed cross thread synchronization and avoid memory failure