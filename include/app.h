#ifndef APP_H
#define APP_H

#include "GLFW/glfw3.h"
#include "vk_process.h"
#include "CGLM/types.h"
#include "dom/dom_entity.h"
#include "background_worker/hashmap_cache_update.h"
#include "windows_utils/file.h"
#include "cache.h"
#include <time.h>
#include <stdio.h>
#include "layout_engine/specs.h"
#include "interpreter/preprocessor.h"
#include "interpreter/tokenizer.h"
#include "interpreter/parser.h"
#include "app_specs.h"

#define VIEW_PORT_WIDTH 1000
#define VIEW_PORT_HEIGHT 760

void window_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    WindowContext *ctx = (WindowContext *)glfwGetWindowUserPointer(window);

    ctx->scroll_x += xoffset / 3;
    ctx->scroll_y += yoffset / 3;
}
static void frame_buffer_size_callback(GLFWwindow *window, int width, int height)
{
    App *app = glfwGetWindowUserPointer(window);
    app->vk_process.window_resized_event = true;
}
static void window_init(App *app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    app->window = glfwCreateWindow(VIEW_PORT_WIDTH, VIEW_PORT_HEIGHT, "Vulkan", NULL, NULL);
    glfwSetWindowUserPointer(app->window, app);
    glfwSetFramebufferSizeCallback(app->window, &frame_buffer_size_callback);
    glfwSetWindowUserPointer(app->window, &app->wctx);
    glfwSetScrollCallback(app->window, &window_scroll_callback);
}
static void vulkan_init(App *app)
{
    instance_create(&app->vk_process);
    messenger_debug_setup(&app->vk_process);
    app->vk_process.surface = malloc(sizeof(VkSurfaceKHR));
    glfwCreateWindowSurface(*app->vk_process.instance, app->window, NULL, app->vk_process.surface);
    physical_device_pick(&app->vk_process, app->gpu_select_flag);
    logical_device_create(&app->vk_process);
    swapchain_create(&app->vk_process, app->window);
    image_views_create(&app->vk_process);
    graphics_pipeline_create(&app->vk_process);
    command_pool_create(&app->vk_process);
    vertex_buffer_create(&app->vk_process);
    instance_buffer_create(&app->vk_process);
    index_buffer_create(&app->vk_process);
    command_buffer_create(&app->vk_process);
    sync_object_create(&app->vk_process);
}
static void dobj_init(App *app)
{
    app->hflo_queue.worker = worker_create();
    CreateThread(NULL, 0, &heap_free_list_organizer, &app->hflo_queue, 0, NULL);

    heap_init((void **)&app->heap, NULL, HEAP_CLASS_HEAP);
    pool_init(&app->pool, NULL, &app->caches);
}
static void main_loop(App *app)
{
    for(;!glfwWindowShouldClose(app->window);)
    {
        glfwPollEvents();
        frame_draw(&app->vk_process, app->window, &app->caches,
            (PCpackage)
            {
                .x_scroll = app->wctx.scroll_x,
                .y_scroll = app->wctx.scroll_y
            }
        );
    }
    vkDeviceWaitIdle(*app->vk_process.logical_device);
}
static void vulkan_destroy(VkProcess *process)
{
    VkDevice *device = process->logical_device;
    vkDestroyBuffer(*device, *process->vertex_buffer, NULL);
    vkFreeMemory(*device, *process->vertex_buffer_gmem, NULL);
    vkDestroyBuffer(*device, *process->index_buffer, NULL);
    vkFreeMemory(*device, *process->index_buffer_gmem, NULL);
    
    vkDestroyCommandPool(*device, *process->command_pool, NULL);

    for (uint8_t i = 0; i < process->image_view_count; i++)
    {
        vkDestroyImageView(*device, process->image_views[i], NULL);
    }
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(*device, process->present_complete_semaphore[i], NULL);
        vkDestroySemaphore(*device, process->render_finished_semaphore[i], NULL);
        vkDestroyBuffer(*device, process->instance_buffer[i], NULL);
        vkFreeMemory(*device, process->instance_buffer_gmem[i], NULL);
        vkDestroyFence(*device, process->fence[i], NULL);
    }
    vkDestroySwapchainKHR(*device, *process->swapchain, NULL);
    vkDestroyShaderModule(*device, *process->shader_module, NULL);
    vkDestroyPipelineLayout(*device, *process->pipeline_layout, NULL);
    vkDestroyPipeline(*device, *process->graphics_pipeline, NULL);
    vkDestroyDevice(*device, NULL);
    vkDestroySurfaceKHR(*process->instance, *process->surface, NULL);
    ((PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*process->instance, "vkDestroyDebugUtilsMessengerEXT"))
    (
        *process->instance,
        process->debug_message,
        NULL
    );
    vkDestroyInstance(*process->instance, NULL);
    free(process->instance);
    free(process->physical_device);
    free(process->logical_device);
    free(process->graphics_queue);
    free(process->surface);
    free(process->swapchain);
    free(process->extent_2d);
    free(process->surface_format);
    free(process->pipeline_layout);
    free(process->graphics_pipeline);
    free(process->images);
    free(process->image_views);
    free(process->command_pool);
    free(process->command_buffers);
    free(process->present_complete_semaphore);
    free(process->render_finished_semaphore);
    free(process->vertex_buffer);
    free(process->vertex_buffer_gmem);
    free(process->index_buffer);
    free(process->index_buffer_gmem);
    
}
static void clean_up(App *app)
{
    vulkan_destroy(&app->vk_process);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}

void run(App *app)
{
    resource_init(&app->caches);
    window_init(app);
    dobj_init(app);

    BHeap *dom_asset;

    char *preproced;
    uint64_t size;
    preprocess(app->args.preproc_argc, app->args.preproc_argv, &preproced, &size);

    char *dst;
    uint64_t dst_size;
    uint8_t *dict;
    uint64_t dict_size;
    tokenize(app->args.tok_argc, app->args.tok_argv, preproced, size, &dst, &dst_size, &dict, &dict_size);

    parse(dst, dict, dict_size, app, &dom_asset);

    feav_layout_compute(app->pool, app->caches.results, app->caches.element_count, 0);
    feav_layout_dump(app->pool, app->caches.results);
    vulkan_init(app);
    main_loop(app);
    clean_up(app);
}

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

#endif