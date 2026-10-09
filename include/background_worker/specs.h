#ifndef BACKGROUND_WORKER_SPECS_H
#define BACKGROUND_WORKER_SPECS_H

#include <windows.h>
#include <stdatomic.h>
#include <stdint.h>
#include "dom/specs.h"

typedef struct {
    uint8_t thread;
    uint8_t core;
} PhysicalDeviceThreadInformation;

typedef struct {
    uint8_t logical_core_count;
    uint8_t physical_core_count;
    PhysicalDeviceThreadInformation physical_device_core_n_thread_relationship[256];
} WorkersProperties;

typedef struct {
    PhysicalDeviceThreadInformation thread_info;
    _Atomic int work;
    void *_Atomic active_target;
    HANDLE event;
    _Atomic bool terminate;
} Worker;

typedef struct {
    void *queue_buffer[256];
    HeapClass heap_class[256];
    _Atomic uint8_t queue_submit_index;
    _Atomic uint8_t queue_worker_index;
    Worker worker;
} BackgroundProcessHFLOqueue;

typedef struct {

} BackgroundProcessLEqueue;

typedef struct {
    BackgroundProcessHFLOqueue *p_queue;
    void *p_target;
    HeapClass class;
} BackgroundProcessHFLOqueueSubmitInfo;

Worker worker_create()
{
    return (Worker){.work = 0, .active_target = NULL, .event = CreateEvent(NULL, FALSE, FALSE, NULL)};
}

void terminate(Worker *w)
{
    atomic_store(&w->terminate, true);
    SetEvent(w->event);
}

void WorkersGetProperties(WorkersProperties *wp)
{
    uint16_t max;
    ARR_SIZE(&max, UINT16_MAX, wp->physical_device_core_n_thread_relationship);

    DWORD len;
    GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info = malloc(len);
    if (!info) return;

    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &len))
    {
        free(info);
        return;
    }
    uint16_t count = 0;
    uint16_t physical_core = 0;

    BYTE *p = (BYTE *)info;
    BYTE *end = p + len;

    while (p < end)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX e = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)p;

        if (e->Relationship == RelationProcessorCore)
        {
            KAFFINITY mask = e->Processor.GroupMask[0].Mask;

            for (uint16_t bit = 0; bit < sizeof(KAFFINITY) * 8; bit++)
            {
                if (mask & ((KAFFINITY)1 << bit))
                {
                    if (count < max)
                    {
                        wp->physical_device_core_n_thread_relationship[count].thread = bit;
                        wp->physical_device_core_n_thread_relationship[count].core = physical_core;
                    }
                    count++;
                }
            }
            physical_core++;
        }
        p += e->Size;
    }
    free(info);
    wp->logical_core_count = count;
    wp->physical_core_count = physical_core;
}

#endif