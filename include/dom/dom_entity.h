#ifndef DOM_ENTITY_H
#define DOM_ENTITY_H

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
    CUSTOM      = 0,//
    HTML        = 1,//
    HEAD        = 2,//
    BODY        = 3,
    TITLE       = 4,//
    LINK        = 5,
    STYLE       = 6,
    SCRIPT      = 7,
    META        = 8,
    DIV         = 9,//
    P           = 10,//
    BR          = 11,
    HR          = 12,
    H1          = 13,//
    H2          = 14,//
    H3          = 15,//
    H4          = 16,//
    H5          = 17,//
    H6          = 18,//
    A           = 19,//
    IMG         = 20,//
    UL          = 21,//
    OL          = 22,//
    LI          = 23,//
    TABLE       = 24,//
    THEAD       = 25,//
    TBODY       = 26,//
    TFOOT       = 27,//
    TR          = 28,//
    TH          = 29,//
    TD          = 30,//
    FORM        = 31,
    DINPUT      = 32,//
    TEXTAREA    = 33,//
    BUTTON      = 34,
    SELECT      = 35,
    OPTION      = 36,
    LABEL       = 37,//
    HEADER      = 38,//
    NAV         = 39,//
    MAIN        = 40,
    SECTION     = 41,
    ARTICLE     = 42,
    ASIDE       = 43,
    FOOTER      = 44,
    VIDEO       = 45,
    AUDIO       = 46,
    SOURCE      = 47,
    CANVAS      = 48,
    SVG         = 49,//
    DETAILS     = 50,
    SUMMARY     = 51,
    IFRAME      = 52,
    PRE         = 53,
    CODE        = 54,
    BLOCKQUOTE  = 55,
    STRONG      = 56,
    EM          = 57,
    B           = 58,
    I           = 59,
    U           = 60,
    SMALL       = 61,
    SUB         = 62,//
    SUP         = 63//
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
    void *p_prev;
    void *p_next;
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

uint8_t tag_name_resolver(char *name)
{
    char name0 = name[0];
    uint8_t len = strlen(name);
    switch (len)
    {
        case 3:
            if (strcmp(name, "div") == 0) return 9;
            else if (strcmp(name, "nav") == 0) return 39;
            else if (strcmp(name, "img") == 0) return 20;
            else if (strcmp(name, "svg") == 0) return 49;
            else if (strcmp(name, "sup") == 0) return 63;
            else if (strcmp(name, "sub") == 0) return 62;
            else if (strcmp(name, "pre") == 0) return 53;
            break;
        case 1:
            switch (name0)
            {
                case 'p': return 10;
                case 'a': return 19;
                case 'b': return 58;
                case 'i': return 59;
                case 'u': return 60;
            }
            break;
        case 2:
            if (strcmp(name, "li") == 0) return 23;
            else if (strcmp(name, "td") == 0) return 30;
            else if (strcmp(name, "br") == 0) return 11;
            else if (strcmp(name, "tr") == 0) return 28;
            else if (strcmp(name, "th") == 0) return 29;
            else if (strcmp(name, "ul") == 0) return 21;
            else if (strcmp(name, "ol") == 0) return 22;
            else if (strcmp(name, "em") == 0) return 57;
            else if (strcmp(name, "hr") == 0) return 12;
            break;
        case 4:
            if (strcmp(name, "body") == 0) return 3;
            else if (strcmp(name, "link") == 0) return 5;
            else if (strcmp(name, "meta") == 0) return 8;
            else if (strcmp(name, "form") == 0) return 31;
            else if (strcmp(name, "main") == 0) return 40;
            else if (strcmp(name, "code") == 0) return 54;
            break;
        case 5:
            if (strcmp(name, "input") == 0) return 32;
            else if (strcmp(name, "label") == 0) return 37;
            else if (strcmp(name, "aside") == 0) return 43;
            else if (strcmp(name, "style") == 0) return 6;
            else if (strcmp(name, "video") == 0) return 45;
            else if (strcmp(name, "audio") == 0) return 46;
            else if (strcmp(name, "small") == 0) return 61;
            break;
        case 6:
            if (strcmp(name, "script") == 0) return 7;
            else if (strcmp(name, "button") == 0) return 34;
            else if (strcmp(name, "select") == 0) return 35;
            else if (strcmp(name, "option") == 0) return 36;
            else if (strcmp(name, "footer") == 0) return 44;
            else if (strcmp(name, "strong") == 0) return 56;
            else if (strcmp(name, "source") == 0) return 47;
            else if (strcmp(name, "canvas") == 0) return 48;
            break;
        case 7:
            if (strcmp(name, "section") == 0) return 41;
            else if (strcmp(name, "article") == 0) return 42;
            break;
    }
    switch (name0) {
        case 'h':
            if (len == 2) {
                switch (name[1])
                {
                    case '1': return 13U;
                    case '2': return 14U;
                    case '3': return 15U;
                    case '4': return 16U;
                    case '5': return 17U;
                    case '6': return 18U;
                }
            }
            if (strcmp(name, "header") == 0) return 38;
            else if (strcmp(name, "head") == 0) return 2;
            else if (strcmp(name, "html") == 0) return 1;
            break;
        case 't':
            if (strcmp(name, "tbody") == 0) return 26;
            else if (strcmp(name, "thead") == 0) return 25;
            else if (strcmp(name, "table") == 0) return 24;
            else if (strcmp(name, "textarea") == 0) return 33;
            else if (strcmp(name, "title") == 0) return 4;
            else if (strcmp(name, "tfoot") == 0) return 27;
            break;
    }
    if (strcmp(name, "blockquote") == 0) return 55;
    else if (strcmp(name, "details") == 0) return 50;
    else if (strcmp(name, "iframe") == 0) return 52;
    else if (strcmp(name, "summary") == 0) return 51;
    return 0;
}

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

#endif