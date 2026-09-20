#ifndef DOM_ENTITY_H
#define DOM_ENTITY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "debugger.h"
#include "hardware_utils/hash.h"

#define QWORD_SCAN_IS_OCCUPIED_MASK 0b0000000100000001000000010000000100000001000000010000000100000001ULL
#define DWORD_SCAN_IS_OCCUPIED_MASK 0b00000001000000010000000100000001UL
#define HALFWORD_SCAN_IS_OCCUPIED_MASK 0b00000001U
#define QWORD_WIDTH 64
#define DWORD_WIDTH 32
#define HALFWORD_WIDTH 8
#define POOL_STATUS_IS_OCCUPIED_BIT 0b00000001U
#define POOL_ELEMENT_COUNT 1024
#define POOL_STATUS_FLAGS_SIZE POOL_ELEMENT_COUNT * HALFWORD_WIDTH
#define HEAP_SIZE 1024 * 128
#define BIG_HEAP_SIZE 1024 * 1024 * 8
#define BIG_BIG_HEAP_SIZE 1024 * 1024 * 512

typedef uint64_t qword;
typedef uint32_t dword;
typedef uint16_t word;
typedef uint8_t hword;

typedef enum {
    UPDATE_DOBJ_FLAG_TAG_NAME         = 0b0000000000000001,
    UPDATE_DOBJ_FLAG_TAG_ID           = 0b0000000000000010,
    UPDATE_DOBJ_FLAG_P_ARENA_ATTR     = 0b0000000000000100,
    UPDATE_DOBJ_FLAG_ARENA_ATTR_SIZE  = 0b0000000000001000,
    UPDATE_DOBJ_FLAG_P_ARENA_INNER    = 0b0000000000010000,
    UPDATE_DOBJ_FLAG_ARENA_INNER_SIZE = 0b0000000000100000,
    UPDATE_DOBJ_FLAG_P_PARENT         = 0b0000000001000000,
    UPDATE_DOBJ_FLAG_LIFE_SPAN_POINT  = 0b0000000010000000
} UpdateDOBJFlags;
typedef enum {
    UPDATE_DOBJ_FLAG_GROUP_BUILD_DOM =
    (
        UPDATE_DOBJ_FLAG_LIFE_SPAN_POINT |
        UPDATE_DOBJ_FLAG_P_ARENA_ATTR |
        UPDATE_DOBJ_FLAG_ARENA_ATTR_SIZE |
        UPDATE_DOBJ_FLAG_P_ARENA_INNER |
        UPDATE_DOBJ_FLAG_ARENA_INNER_SIZE
    ),
} UpdateDOBJFlagGroups;

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
    void *heap;
    uint16_t a;
    uint16_t s;
} HeapHandler16;
typedef struct {
    uint16_t a;
    uint16_t s;
} HeapChunkHandler16;
typedef struct {
    void *heap;
    uint64_t a;
    uint64_t s;
} HeapHandler64;
typedef struct {
    uint64_t a;
    uint64_t s;
} HeapChunkHandler64;

typedef struct {
    uint16_t heap_index;
    uint16_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler16 *free_list;
    uint16_t free_list_count;
    uint16_t heap_free_list_cache[HEAP_SIZE / 2];
    uint8_t heap_status;

    uint16_t mem[HEAP_SIZE / 2]; //size: 128KB, page size: 2B, (heap size / minimum allocatable size) = UINT16_MAX
} Heap;
typedef struct {
    uint16_t heap_index;
    uint8_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler64 *free_list;
    uint8_t free_list_count;
    uint8_t heap_free_list_cache[256];
    uint8_t heap_status;

    uint64_t mem[BIG_HEAP_SIZE / 8]; //size: 8MB, page size: 8B, (heap size / minimum allocatable size) < UINT8_MAX
} BHeap;
typedef struct {
    uint16_t heap_index;
    uint8_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler64 *free_list;
    uint8_t free_list_count;
    uint8_t heap_free_list_cache[256];
    uint8_t heap_status;

    uint64_t mem[BIG_BIG_HEAP_SIZE / 8]; //size: 512MB, page size: 8B, (heap size / minimum allocatable size) < UINT8_MAX
} BBHeap;

typedef enum {
    HEAP_CLASS_HEAP = 1,
    HEAP_CLASS_BIG_HEAP = 2,
    HEAP_CLASS_BIG_BIG_HEAP = 3
} HeapClass;

typedef struct {
    char tag_name[256];
    TagIdentifier tag_id;
    void *p_attr;
    size_t attr_size;
    void *p_inner;
    size_t inner_size;
    void *p_parent;
    uint8_t life_span_point;
} DOBJ;

typedef struct {
    uint16_t pool_index;
    uint16_t live_objects;
    void *p_prev;
    void *p_next;
    uint8_t status_flags[POOL_ELEMENT_COUNT];

    char tag_names[POOL_ELEMENT_COUNT][256];
    TagIdentifier tag_ids[POOL_ELEMENT_COUNT];
    void *p_attrs[POOL_ELEMENT_COUNT];
    size_t attr_sizes[POOL_ELEMENT_COUNT];
    void *p_inner[POOL_ELEMENT_COUNT];
    size_t inner_size[POOL_ELEMENT_COUNT];
    void *p_parent[POOL_ELEMENT_COUNT];
    uint8_t life_span_points[POOL_ELEMENT_COUNT];
} DOBJPool;

typedef struct {
    DOBJPool *pool;
    uint16_t item_index;
} DOBJHandler;



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

void pool_init(DOBJPool **pool, DOBJPool *prev_pool) //latest pool must be passed in
{
    *pool = malloc(sizeof(DOBJPool));

    (*pool)->live_objects = 0;

    if (prev_pool != NULL)
    {
        (*pool)->p_prev = prev_pool;
        prev_pool->p_next = *pool;
        (*pool)->pool_index = prev_pool->pool_index + 1;
    } else
    {
        (*pool)->p_prev = NULL;
        (*pool)->pool_index = 0;
    }
    (*pool)->p_next = NULL;
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
uint64_t dobj_pool_alloc(DOBJ obj, DOBJPool *pool)
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
    working_pool->p_attrs[working_index] = obj.p_attr;
    working_pool->attr_sizes[working_index] = obj.attr_size;
    working_pool->p_inner[working_index] = obj.p_inner;
    working_pool->inner_size[working_index] = obj.inner_size;
    working_pool->p_parent[working_index] = obj.p_parent;
    working_pool->life_span_points[working_index] = obj.life_span_point;

    return working_pool->pool_index * POOL_ELEMENT_COUNT + working_index;
}

void dobj_pool_update(DOBJ obj, DOBJPool *pool, uint64_t glb_index, uint16_t opt_flags)
{
    DOBJHandler h = pool_search(pool, glb_index);
    if (!(h.pool->status_flags[h.item_index] & POOL_STATUS_IS_OCCUPIED_BIT))
    {
        printf(ERR SYS_DBG_PREFIX" Segmentation fault, invalid pool item access");
        exit(1);
    }

}

void dobj_pool_free(DOBJPool *pool, uint64_t pool_glb_index)
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

#define HEAP_INIT(heap, prev_heap) \
{ \
    (heap)->live_objects = 0; \
    (heap)->free_list_count = 0; \
    (heap)->free_list = NULL; \
    if (prev_heap != NULL) \
    { \
        (heap)->p_prev = prev_heap; \
        prev_heap->p_next = heap; \
        (heap)->heap_index = prev_heap->heap_index + 1; \
    } else \
    { \
        (heap)->p_prev = NULL; \
        (heap)->heap_index = 0; \
    } \
    (heap)->p_next = NULL; \
}

void heap_init(void **heap, void *prev_heap, uint8_t heap_class)
{
    switch (heap_class)
    {
        case HEAP_CLASS_HEAP:
            *heap = malloc(sizeof(Heap));

            Heap *h = (Heap *)*heap;
            Heap *ph = (Heap *)prev_heap;

            HEAP_INIT(h, ph);
            break;
        case HEAP_CLASS_BIG_HEAP:
            *heap = malloc(sizeof(BHeap));

            BHeap *h = (BHeap *)*heap;
            BHeap *ph = (BHeap *)prev_heap;

            HEAP_INIT(h, ph);
            break;
        case HEAP_CLASS_BIG_BIG_HEAP:
            *heap = malloc(sizeof(BBHeap));

            BBHeap *h = (BBHeap *)*heap;
            BBHeap *ph = (BBHeap *)prev_heap;

            HEAP_INIT(h, ph);
            break;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class initialization");
            exit(1);
    }
}

HeapHandler64 heap_free(HeapHandler64 hh, uint8_t heap_class)
{
    switch (heap_class)
    {
        case HEAP_CLASS_HEAP:
            Heap *h = (Heap *)hh.heap;


            break;
        case HEAP_CLASS_BIG_HEAP:
            BHeap *h = (BHeap *)hh.heap;


            break;
        case HEAP_CLASS_BIG_BIG_HEAP:
            BBHeap *h = (BBHeap *)hh.heap;


            break;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class deallocation");
            exit(1);
    }
}
HeapHandler64 heap_realloc(HeapHandler64 hh, size_t size, uint8_t heap_class, Worker *w)
{
    switch (heap_class)
    {
        case HEAP_CLASS_HEAP:
            Heap *h = (Heap *)hh.heap;

            size_t after_lc_addr = hh.a + hh.s;
            uint16_t ind = hash_2_index(crc32(after_lc_addr), UINT16_MAX);
            HeapChunkHandler16 ch = h->free_list[ind];
            bool is_valid_extend = ch.a == after_lc_addr && hh.s + ch.s >= size;
            bool outdated_hashmap = w->active_target == h;
            
            if (is_valid_extend && !outdated_hashmap)
            {
                //run algirthm to recalculate
            } else {
                //scan free list 
            }
            
            break;
        case HEAP_CLASS_BIG_HEAP:
            BHeap *h = (BHeap *)hh.heap;


            break;
        case HEAP_CLASS_BIG_BIG_HEAP:
            BBHeap *h = (BBHeap *)hh.heap;


            break;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class access");
            exit(1);
    }
}
#endif