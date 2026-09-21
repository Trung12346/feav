#ifndef BACKGROUND_WORKER_SPECS_H
#define BACKGROUND_WORKER_SPECS_H

#include <windows.h>
#include <stdatomic.h>
#include <stdint.h>
#include "dom/specs.h"

typedef struct {
    _Atomic int work;
   void *_Atomic active_target;
    HANDLE event;
} Worker;
typedef struct {
    void *queue_buffer[256];
    HeapClass heap_class[256];
    _Atomic uint8_t queue_submit_index;
    _Atomic uint8_t queue_worker_index;
    Worker worker;
} BackgroundProcessQueue;

typedef struct {
    BackgroundProcessQueue *p_queue;
    void *p_target;
    HeapClass class;
} BackgroundProcessQueueSubmitInfo;

#endif