#ifndef HASHMAP_CACHE_UPDATE_H
#define HASHMAP_CACHE_UPDATE_H

#include <windows.h>
#include <stdatomic.h>
#include <stdint.h>
#include "background_worker/specs.h"
#include "dom/specs.h"
#include "debugger.h"
#include "main.h"
#include "hardware_utils/hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Worker worker_create()
{
    return (Worker){.work = 0, .active_target = NULL, .event = CreateEvent(NULL, FALSE, FALSE, NULL)};
}

#define HCH_INSERTION_THRESHOLD 32

/* ---- Macro template: generates all functions for a given type ---- */
#define DEFINE_MERGE_SORT(SUFFIX, TYPE) \
static void insertion_sort_##SUFFIX(TYPE *arr, uint64_t left, uint64_t right) \
{ \
    for (uint64_t i = left + 1; i < right; i++) \
    { \
        TYPE key = arr[i]; \
        uint64_t j = i; \
        while (j > left && arr[j - 1].s > key.s) \
        { \
            arr[j] = arr[j - 1]; \
            j--; \
        } \
        arr[j] = key; \
    } \
} \
static void merge_run_##SUFFIX( \
    const TYPE *src, \
    TYPE *dst, \
    uint64_t left, \
    uint64_t mid, \
    uint64_t right \
) \
{ \
    if (src[mid - 1].s <= src[mid].s) \
    { \
        memcpy(dst + left, src + left, (right - left) * sizeof(*dst)); \
        return; \
    } \
    uint64_t i = left, j = mid, k = left; \
    while (i < mid && j < right) \
        dst[k++] = (src[i].s <= src[j].s) ? src[i++] : src[j++]; \
    while (i < mid) \
        dst[k++] = src[i++]; \
    while (j < right) \
        dst[k++] = src[j++]; \
} \
int merge_sort_snapshot_##SUFFIX(TYPE *array, uint64_t count) \
{ \
    if (count < 2) \
        return 1; \
    TYPE *bufA = malloc(count * sizeof(*bufA)); \
    TYPE *bufB = malloc(count * sizeof(*bufB)); \
    if (bufA == NULL || bufB == NULL) \
    { \
        free(bufA); \
        free(bufB); \
        return 0; \
    } \
    memcpy(bufA, array, count * sizeof(*bufA)); \
    for (uint64_t start = 0; start < count; start += HCH_INSERTION_THRESHOLD) \
    { \
        uint64_t end = start + HCH_INSERTION_THRESHOLD; \
        if (end > count) \
            end = count; \
        insertion_sort_##SUFFIX(bufA, start, end); \
    } \
    TYPE *src = bufA; \
    TYPE *dst = bufB; \
    for (uint64_t width = HCH_INSERTION_THRESHOLD; width < count; width *= 2) \
    { \
        uint64_t left = 0; \
        for (; left < count; left += 2 * width) \
        { \
            uint64_t mid = left + width; \
            uint64_t right = left + 2 * width; \
            if (mid >= count) \
            { \
                memcpy(dst + left, src + left, \
                       (count - left) * sizeof(*dst)); \
                break; \
            } \
            if (right > count) \
                right = count; \
            merge_run_##SUFFIX(src, dst, left, mid, right); \
        } \
        TYPE *swap = src; \
        src = dst; \
        dst = swap; \
    } \
    memcpy(array, src, count * sizeof(*array)); \
    free(bufA); \
    free(bufB); \
    return 1; \
}
DEFINE_MERGE_SORT(16, HeapChunkHandler16)
DEFINE_MERGE_SORT(64, HeapChunkHandler64)

#define HASHMAP_REBUILD(SIZE, HEAP) \
{ \
    merge_sort_snapshot_##SIZE(HEAP->free_list, HEAP->free_list_count); \
    for (uint##SIZE##_t i = 0; i < HEAP->free_list_count; i++) \
    { \
        uint##SIZE##_t h_hash_ind = hash_2_index(hash64(HEAP->free_list[i].a), UINT##SIZE##_MAX); \
        HEAP->heap_free_list_cache_h[h_hash_ind] = i; \
        uint##SIZE##_t t_hash_ind = hash_2_index(hash64(HEAP->free_list[i].a + HEAP->free_list[i].s), UINT##SIZE##_MAX); \
        HEAP->heap_free_list_cache_t[t_hash_ind] = i; \
    } \
}

DWORD WINAPI heap_free_list_organizer(void *arg)
{
    BackgroundProcessQueue *queue = (BackgroundProcessQueue *)arg;
    Worker *w = &queue->worker;
    for (;;)
    {
        WaitForSingleObject(w->event, INFINITE);
        if (atomic_exchange(&w->work, 0))
        {
            for (; queue->queue_submit_index - queue->queue_worker_index != 0;)
            {
                atomic_fetch_add(&queue->queue_worker_index, 1);
                uint8_t working_ind = queue->queue_worker_index;
                atomic_store(&w->active_target, queue->queue_buffer[working_ind]);
                switch (queue->heap_class[working_ind])
                {
                    case HEAP_CLASS_HEAP:
                        Heap *h = w->active_target;

                        // merge_sort_snapshot_16(h->free_list, h->free_list_count);
                        // for (uint16_t i = 0; i < h->free_list_count; i++)
                        // {
                        //     uint16_t hash_ind = hash_2_index(hash64(h->free_list[i].a), UINT16_MAX);
                        //     h->heap_free_list_cache[hash_ind] = i;
                        // }
                        HASHMAP_REBUILD(16, h);
                        break;
                    case HEAP_CLASS_BIG_HEAP:
                        BHeap *bh = w->active_target;

                        HASHMAP_REBUILD(64, bh);
                        break;
                    case HEAP_CLASS_BIG_BIG_HEAP:
                        BBHeap *bbh = w->active_target;

                        HASHMAP_REBUILD(64, bbh);
                }

            }
            atomic_store(&w->active_target, NULL);
        }
    }

    return 0;
}
void request_hflo(BackgroundProcessQueueSubmitInfo info)
{
    Worker *w = &info.p_queue->worker;
    BackgroundProcessQueue *queue = info.p_queue;
    atomic_store(&w->work, 1);
    atomic_fetch_add(&queue->queue_submit_index, 1);
    if (queue->queue_submit_index == queue->queue_worker_index)
    {
        printf(ERR SYS_DBG_PREFIX" Background process queue buffer overflowed");
        exit(1);
    }
    atomic_store(&queue->queue_buffer[queue->queue_submit_index], info.p_target);
    atomic_store(&queue->heap_class[queue->queue_submit_index], info.class);
    SetEvent(w->event);
}
//future note: steal vulkan queue so worker is capable of handling awaiting requests appropriately too
#endif