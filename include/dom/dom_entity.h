#ifndef DOM_ENTITY_H
#define DOM_ENTITY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "debugger.h"
#include "background_worker/hashmap_cache_update.h"
#include <stdatomic.h>
#include "dom/specs.h"
#include "hardware_utils/hash.h"

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

#define HEAP_INIT(heap, prev_heap, handler_size) \
{ \
    (heap)->live_objects = 0; \
    (heap)->free_list_count = 1; \
    (heap)->free_list_alloc_count = FREE_LIST_ALLOC_COUNT; \
    (heap)->free_list = malloc(sizeof(HeapChunkHandler##handler_size) * FREE_LIST_ALLOC_COUNT); \
    (heap)->free_list[0] = (HeapChunkHandler##handler_size){0, 0}; \
    ARR_SIZE(&(heap)->free_list[0].s, UINT64_MAX, (heap)->free_list) \
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

            HEAP_INIT(h, ph, 16);
            break;
        case HEAP_CLASS_BIG_HEAP:
            *heap = malloc(sizeof(BHeap));

            BHeap *bh = (BHeap *)*heap;
            BHeap *bph = (BHeap *)prev_heap;

            HEAP_INIT(bh, bph, 64);
            break;
        case HEAP_CLASS_BIG_BIG_HEAP:
            *heap = malloc(sizeof(BBHeap));

            BBHeap *bbh = (BBHeap *)*heap;
            BBHeap *bbph = (BBHeap *)prev_heap;

            HEAP_INIT(bbh, bbph, 64);
            break;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class initialization");
            exit(1);
    }
}


#define HEAP_ALLOC(HEAP, CH_TYPE) \
{ \
    if (size > sizeof(HEAP->mem)) \
    { \
        printf(ERR SYS_DBG_PREFIX" Heap chunk overflow"); \
        exit(1); \
    } \
    size = size / sizeof(HEAP->mem[0]); \
    rtn_obj.s = size; \
    bool chunk_found = false; \
    for (uint##CH_TYPE##_t i = 0; i < HEAP->free_list_count; i++) \
    { \
        HeapChunkHandler##CH_TYPE *ch = &HEAP->free_list[i]; \
        if (ch->s > size) \
        { \
            rtn_obj.heap = HEAP; \
            rtn_obj.a = ch->a; \
            ch->a += size; \
            ch->s -= size; \
            chunk_found = true; \
            break; \
        } else if (ch->s == size) \
        { \
            rtn_obj.heap = HEAP; \
            rtn_obj.a = ch->a; \
            *ch = HEAP->free_list[--HEAP->free_list_count]; \
            chunk_found = true; \
            break; \
        } \
    } \
    if (!chunk_found) \
    { \
        if (HEAP->p_next == NULL) \
        { \
            heap_init(&HEAP->p_next, HEAP, heap_class); \
        } \
        rtn_obj = heap_alloc(HEAP->p_next, size, heap_class, queue); \
        HEAP->live_objects--; \
    } \
    queue_submit_hflo \
    ( \
        (BackgroundProcessQueueSubmitInfo) \
        { \
            .class = heap_class, \
            .p_target = HEAP, \
            .p_queue = queue \
        } \
    ); \
}
HeapHandler64 heap_alloc(void *heap, size_t size, uint8_t heap_class, BackgroundProcessQueue *queue)
{
    HeapHandler64 rtn_obj = {0};
    switch (heap_class)
    {
        case HEAP_CLASS_HEAP:
            Heap *h = (Heap *)heap;

            HEAP_ALLOC(h, 16);
            return rtn_obj;
        case HEAP_CLASS_BIG_HEAP:
            BHeap *bh = (BHeap *)heap;

            HEAP_ALLOC(bh, 64);
            return rtn_obj;
        case HEAP_CLASS_BIG_BIG_HEAP:
            BBHeap *bbh = (BBHeap *)heap;

            HEAP_ALLOC(bbh, 64);
            return rtn_obj;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class allocation");
            exit(1);
    }
}

#define HEAP_FREE(HEAP, ALIAS, CH_TYPE) \
{ \
    uint##CH_TYPE##_t hash_ind_h = hash_2_index(hash64(hh.a + hh.s), UINT##CH_TYPE##_MAX); \
    uint##CH_TYPE##_t hash_ind_t = hash_2_index(hash64(hh.a), UINT##CH_TYPE##_MAX); \
    HeapChunkHandler##CH_TYPE *head_chunk = &HEAP->free_list[HEAP->heap_free_list_cache_h[hash_ind_h]]; \
    HeapChunkHandler##CH_TYPE *tail_chunk = &HEAP->free_list[HEAP->heap_free_list_cache_t[hash_ind_t]]; \
    bool outdated_hashmap = (ALIAS *)w->active_target == HEAP; \
    bool is_head_free = head_chunk->a == hh.a + hh.s && !outdated_hashmap; \
    bool is_tail_free = tail_chunk->a + tail_chunk->s == hh.a && !outdated_hashmap; \
    bool merge_nd = false; \
    HeapChunkHandler##CH_TYPE buffer; \
    if (is_head_free) \
    { \
        buffer = *head_chunk; \
        buffer.a = hh.a; \
        buffer.s += hh.s; \
        merge_nd = true; \
    } \
    if (is_tail_free) \
    { \
        if (merge_nd) \
        { \
            buffer.a = tail_chunk->a; \
            buffer.s += tail_chunk->s; \
            *tail_chunk = HEAP->free_list[--HEAP->free_list_count]; \
            *head_chunk = buffer; \
        } else \
        { \
            tail_chunk->s += hh.s; \
        } \
    } \
    if (!is_head_free && !is_tail_free) \
    { \
        HEAP->free_list[HEAP->free_list_count++] = (HeapChunkHandler##CH_TYPE) \
        { \
            .a = hh.a, \
            .s = hh.s \
        }; \
    } \
    queue_submit_hflo \
    ( \
        (BackgroundProcessQueueSubmitInfo) \
        { \
            .p_queue = queue, \
            .p_target = HEAP, \
            .class = heap_class \
        } \
    ); \
}

HeapHandler64 heap_free(HeapHandler64 hh, uint8_t heap_class, BackgroundProcessQueue *queue)
{
    Worker *w = &queue->worker;
    switch (heap_class)
    {
        case HEAP_CLASS_HEAP:
            Heap *h = (Heap *)hh.heap;

            HEAP_FREE(h, Heap, 16);
            break;
        case HEAP_CLASS_BIG_HEAP:
            BHeap *bh = (BHeap *)hh.heap;

            HEAP_FREE(bh, BHeap, 64);
            break;
        case HEAP_CLASS_BIG_BIG_HEAP:
            BBHeap *bbh = (BBHeap *)hh.heap;

            HEAP_FREE(bbh, BBHeap, 64);
            break;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class deallocation");
            exit(1);
    }
}

#define HEAP_REALLOC(HEAP, ALIAS, CH_TYPE) \
{ \
    if (size > sizeof(HEAP->mem)) \
    { \
        printf(ERR SYS_DBG_PREFIX" Heap chunk overflow"); \
        exit(1); \
    } \
    size = size / sizeof(HEAP->mem[0]); \
    uint##CH_TYPE##_t after_lc_addr = hh.a + hh.s; \
    uint##CH_TYPE##_t hash_ind = hash_2_index(hash64(after_lc_addr), UINT##CH_TYPE##_MAX); \
    HeapChunkHandler##CH_TYPE *p_ch = &HEAP->free_list[HEAP->heap_free_list_cache_h[hash_ind]]; \
    HeapChunkHandler##CH_TYPE ch = *p_ch; \
    bool is_shrink = size < hh.s; \
    bool after_free_present = ch.a == after_lc_addr; \
    bool is_valid_extend = after_free_present && hh.s + ch.s >= size; \
    bool outdated_hashmap = (ALIAS *)w->active_target == HEAP; \
    if (is_shrink && after_free_present) \
    { \
        p_ch->a = hh.a + size; \
        p_ch->s += (hh.s - size); \
        queue_submit_hflo \
        ( \
        (BackgroundProcessQueueSubmitInfo) \
            { \
                .class = heap_class, \
                .p_target = HEAP, \
                .p_queue = queue \
            } \
        ); \
        return rtn_obj; \
    } else if (is_valid_extend && !outdated_hashmap) \
    { \
        size_t offset = (size - hh.s); \
        if (offset == p_ch->s) \
        { \
            *p_ch = HEAP->free_list[--HEAP->free_list_count]; \
        } else { \
            p_ch->a += offset; \
            p_ch->s -= offset; \
        } \
        queue_submit_hflo \
        ( \
            (BackgroundProcessQueueSubmitInfo) \
            { \
                .class = heap_class, \
                .p_target = HEAP, \
                .p_queue = queue \
            } \
        ); \
        return rtn_obj; \
    } else \
    { \
        bool chunk_found = false; \
        for (uint##CH_TYPE##_t i = 0; i < HEAP->free_list_count; i++) \
        { \
            if (HEAP->free_list[i].s > size) \
           { \
                rtn_obj.a = HEAP->free_list[i].a; \
                HEAP->free_list[i].a += size; \
                HEAP->free_list[i].s -= size; \
                chunk_found = true; \
                break; \
            } else if (HEAP->free_list[i].s == size) \
            { \
                rtn_obj.a = HEAP->free_list[i].a; \
                HEAP->free_list[i] = HEAP->free_list[--HEAP->free_list_count]; \
                chunk_found = true; \
                break; \
            } \
        } \
        if (!chunk_found) \
        { \
            if (HEAP->p_next == NULL) \
            { \
                heap_init(&HEAP->p_next, HEAP, heap_class); \
                ALIAS *nh = (ALIAS *)HEAP->p_next; \
                rtn_obj = heap_alloc(nh, size, heap_class, queue); \
            } \
            HEAP->live_objects--; \
        } \
        heap_free(hh, heap_class, queue); \
        return rtn_obj; \
    } \
}

HeapHandler64 heap_realloc(HeapHandler64 hh, size_t size, uint8_t heap_class, BackgroundProcessQueue *queue)
{
    if (hh.s == size) return hh;
    HeapHandler64 rtn_obj = hh;
    rtn_obj.s = size;
    Worker *w = &queue->worker;
    switch (heap_class)
    {
        case HEAP_CLASS_HEAP:
            Heap *h = (Heap *)hh.heap;

            HEAP_REALLOC(h, Heap, 16);
            break;
        case HEAP_CLASS_BIG_HEAP:
            BHeap *bh = (BHeap *)hh.heap;

            HEAP_REALLOC(bh, BHeap, 64);
            break;
        case HEAP_CLASS_BIG_BIG_HEAP:
            BBHeap *bbh = (BBHeap *)hh.heap;

            HEAP_REALLOC(bbh, BBHeap, 64);
            break;
        default:
            printf(ERR SYS_DBG_PREFIX" Undefined heap class access");
            exit(1);
    }
    return rtn_obj;
}
#endif