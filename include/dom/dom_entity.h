#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

#define QWORD_SCAN_IS_FREE_MASK 0b0000000100000001000000010000000100000001000000010000000100000001ULL
#define DWORD_SCAN_IS_FREE_MASK 0b00000001000000010000000100000001UL
#define HALFWORD_SCAN_IS_FREE_MASK 0b00000001U
#define QWORD_WIDTH 64
#define DWORD_WIDTH 32
#define HALFWORD_WIDTH 8
#define POOL_SIZE 1024 * 64
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
    uint8_t status_flags[1024];

    char tag_names[1024][32];
    TagIdentifier tag_ids[1024];
    uintptr_t p_arena_attrs[1024];
    size_t arena_attr_sizes[1024];
    uintptr_t p_arena_inner[1024];
    size_t arena_inner_size[1024];
    uintptr_t p_parent[1024];
    uint8_t life_span_points[1024];
} DOBJPool;

typedef struct {
    uint16_t arena_index;
    uint16_t *size_list;
    uint16_t size_list_size;
    uint16_t live_objects;
    uintptr_t p_prev;
    uintptr_t p_next;

    uint8_t mem[POOL_SIZE]; //size: 64KB, page size: 1B
} Arena;
typedef struct {
    uint16_t arena_index;
    uint24_t *size_list;
    uint16_t size_list_size;
    uint16_t live_objects;
    uintptr_t p_prev;
    uintptr_t p_next;

    uint64_t mem[8 * 1024 * 1024]; //size: 64MB, page size: 8B
} BigArena;

uint16_t available_pool(DOBJPool *pool, DOBJ obj, bool *free_found)
{
    *free_found = false;
    for (uint32_t i = 0; i < POOL_SIZE / QWORD_WIDTH; i++)
    {
        uint64_t buffer;
        uint32_t sbuffer;
        uint8_t ssbuffer;
        memcpy(&buffer, pool->status_flags + i * 8, 8);
        if (buffer & QWORD_SCAN_IS_FREE_MASK)
        {
            for (uint8_t si = 0; si < QWORD_WIDTH / DWORD_WIDTH; si++)
            {
                memcpy(&sbuffer, pool->status_flags + i * 8 + si * 4, 4);
                if (sbuffer & DWORD_SCAN_IS_FREE_MASK)
                {
                    for (uint8_t ssi = 0; ssi < DWORD_WIDTH / HALFWORD_WIDTH; ssi++)
                    {
                        memcpy(&ssbuffer, pool->status_flags + i * 8 + si * 4 + ssi, 1);
                        if (ssbuffer & HALFWORD_SCAN_IS_FREE_MASK)
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
static void dobj_pool_alloc(size_t index, DOBJ *pool)
{
    
}