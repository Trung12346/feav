#ifndef HASHMAP_CACHE_UPDATE_H
#define HASHMAP_CACHE_UPDATE_H

#include <windows.h>
#include <stdatomic.h>
#include "app.h"

typedef struct {
    _Atomic int work;
    _Atomic void *active_target;
    HANDLE event;
} Worker;

Worker worker_create()
{
    return (Worker){.work = 0, .active_target = NULL, .event = CreateEvent(NULL, FALSE, FALSE, NULL)};
}

DWORD WINAPI heap_free_list_organizer(App *app)
{
    for (;;)
    {
        Worker *w = &app->hflo_worker;
        WaitForSingleObject(w->event, INFINITE);
        if (atomic_exchange(&w->work, 0))
        {

            atomic_store(&w->active_target, NULL);
        }
    }

    return 0;
}
void request_hflo(Worker *w, void *target)
{
    atomic_store(&w->active_target, target);
    atomic_store(&w->work, 1);
    SetEvent(w->event);
}
//future note: steal vulkan queue so worker is capable of handling awaiting requests appropriately too
#endif