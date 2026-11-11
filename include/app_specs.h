#ifndef APP_SPECS_H
#define APP_SPECS_H

#include "GLFW/glfw3.h"
#include "vk_process.h"
#include "CGLM/types.h"
#include "dom/dom_entity.h"
#include "background_worker/hashmap_cache_update.h"
#include "windows_utils/file.h"
#include "cache.h"

typedef struct
{
    uint8_t preproc_argc;
    char *preproc_argv[2];

    uint8_t tok_argc;
    char *tok_argv[2];
} AppArguments;

typedef struct
{
    GLFWwindow *window;
    VkProcess vk_process;
    int gpu_select_flag;
    DOBJPool *pool;
    Heap *heap;
    BackgroundProcessHFLOqueue hflo_queue;

    AppCaches caches;
    AppArguments args;
} App;
App app_no_args_construct()
{
    return (App)
    {
        .gpu_select_flag = -1,
        .vk_process = vk_process_no_args_construct(),
        .caches = {0}
    };
}

#endif