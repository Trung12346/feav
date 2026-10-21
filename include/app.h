#ifndef APP_H
#define APP_H

#include "GLFW/glfw3.h"
#include "vk_process.h"
#include "CGLM/types.h"
#include "dom/dom_entity.h"
#include "background_worker/hashmap_cache_update.h"
#include "windows_utils/file.h"

#define VIEW_PORT_WIDTH 1000
#define VIEW_PORT_HEIGHT 760

typedef struct {
    CSSattribute *ca; //"runtime-enum", enum configuration is dependent on hashing algorithm
    HTMLtag *ha;
} AppCaches;

typedef struct
{
    GLFWwindow *window;
    VkProcess vk_process;
    int gpu_select_flag;
    DOBJPool *pool;
    Heap *heap;
    BackgroundProcessHFLOqueue hflo_queue;

    AppCaches caches;
} App;
App app_no_args_construct()
{
    return (App)
    {
        .gpu_select_flag = -1,
        .vk_process = vk_process_no_args_construct()
    };
}
static void frame_buffer_size_callback(GLFWwindow *window, int width, int height)
{
    App *app = glfwGetWindowUserPointer(window);
    app->vk_process.window_resized_event = true;
}
static void window_init(GLFWwindow **window, App *app)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    *window = glfwCreateWindow(VIEW_PORT_WIDTH, VIEW_PORT_HEIGHT, "Vulkan", NULL, NULL);
    glfwSetWindowUserPointer(*window, app);
    glfwSetFramebufferSizeCallback(*window, &frame_buffer_size_callback);
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
    index_buffer_create(&app->vk_process);
    command_buffer_create(&app->vk_process);
    sync_object_create(&app->vk_process);
}
static void dobj_init(App *app)
{
    app->hflo_queue.worker = worker_create();
    CreateThread(NULL, 0, &heap_free_list_organizer, &app->hflo_queue, 0, NULL);

    heap_init((void **)&app->heap, NULL, HEAP_CLASS_HEAP);
    pool_init(&app->pool, NULL);
}
static void main_loop(App *app)
{
    for(;!glfwWindowShouldClose(app->window);)
    {
        glfwPollEvents();
        frame_draw(&app->vk_process, app->window);
    }
    vkDeviceWaitIdle(*app->vk_process.logical_device);
}
static void vulkan_destroy(VkProcess *process)
{
    VkDevice *device = process->logical_device;
    vkDestroyBuffer(*device, *process->vertex_buffer, NULL);
    vkDestroyBuffer(*device, *process->index_buffer, NULL);
    vkDestroyCommandPool(*device, *process->command_pool, NULL);
    vkDestroySemaphore(*device, *process->present_complete_semaphore, NULL);
    vkDestroySemaphore(*device, *process->render_finished_semaphore, NULL);
    vkDestroyImage(*device, *process->images, NULL);
    vkDestroyImageView(*device, *process->image_views, NULL);
    vkDestroySwapchainKHR(*device, *process->swapchain, NULL);
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

void create_html_hmap(App *app, size_t *size, void **buffer)
{
    file_read("../assets/html-tags.bin", size, NULL);
    if (*size == 0)
    {
        printf(ERR SYS_DBG_PREFIX" Missing resource \"html-tags.bin\"");
        exit(1);
    }
    *buffer = malloc(*size);
    file_read("../assets/html-tags.bin", size, *buffer);

    const char *cursor_resrc_p = *buffer;
    uint16_t *cursor_struct_p = (uint16_t *)app->caches.ha;
    for (uint8_t i = 0U; i < HTML_TAG_COUNT; i++)
    {
        *cursor_struct_p = html_tag_hmap_id_get(cursor_resrc_p);

        cursor_struct_p++;
        cursor_resrc_p += HTML_TAG_RESRC_STRIDE;
    }
    void *write_buffer = malloc(sizeof(HTMLtag) + SEED_SIGNATURE_WIDTH);
    *(uint64_t *)write_buffer = SEED_HTML_TAG;
    memcpy(write_buffer + SEED_SIGNATURE_WIDTH, app->caches.ha, sizeof(HTMLtag));
    create_write_file("../assets/html-tags-enum.bin", write_buffer, sizeof(HTMLtag) + SEED_SIGNATURE_WIDTH);
}
void create_css_hmap(App *app, size_t *size, void **buffer)
{
    file_read("../assets/css-attributes.bin", size, NULL);
    if (*size == 0)
    {
        printf(ERR SYS_DBG_PREFIX" Missing resource \"css-attributes.bin\"");
        exit(1);
    }
    *buffer = malloc(*size);
    file_read("../assets/css-attributes.bin", size, *buffer);

    const char *cursor_resrc_p = *buffer;
    uint16_t *cursor_struct_p = (uint16_t *)app->caches.ca;
    for (uint8_t i = 0U; i < CSS_ATTRIBUTE_COUNT; i++)
    {
        *cursor_struct_p = css_attr_hmap_id_get(cursor_resrc_p);

        cursor_struct_p++;
        cursor_resrc_p += CSS_ATTRIBUTE_RESRC_STRIDE;
    }
    void *write_buffer = malloc(sizeof(CSSattribute) + SEED_SIGNATURE_WIDTH);
    *(uint64_t *)write_buffer = SEED_CSS_ATTRIBUTE;
    memcpy(write_buffer + SEED_SIGNATURE_WIDTH, app->caches.ca, sizeof(CSSattribute));
    create_write_file("../assets/css-attributes-enum.bin", write_buffer, sizeof(CSSattribute) + SEED_SIGNATURE_WIDTH);
}
void resource_init(App *app)
{
    app->caches.ha = calloc(1, sizeof(HTMLtag));

    size_t size = 0;
    void *buffer;

    file_read("../assets/html-tags-enum.bin", &size, NULL);

    if (size == 0) create_html_hmap(app, &size, &buffer);
    else
    {
        buffer = malloc(size);
        file_read("../assets/html-tags-enum.bin", &size, buffer);
        if (*(uint64_t *)buffer != SEED_HTML_TAG || size != (sizeof(HTMLtag) + SEED_SIGNATURE_WIDTH))
        {
            free(buffer);
            size = 0;
            buffer = NULL;
            create_html_hmap(app, &size, &buffer);
        } else memcpy(app->caches.ha, buffer + SEED_SIGNATURE_WIDTH, sizeof(HTMLtag));
    }

    free(buffer);
    buffer = NULL;
    size = 0;

    app->caches.ca = calloc(1, sizeof(CSSattribute));
    file_read("../assets/css-attributes-enum.bin", &size, NULL);

    if (size == 0) create_css_hmap(app, &size, &buffer);
    else
    {
        buffer = malloc(size);
        file_read("../assets/css-attributes-enum.bin", &size, buffer);
        if (*(uint64_t *)buffer != SEED_CSS_ATTRIBUTE || size != (sizeof(CSSattribute) + SEED_SIGNATURE_WIDTH))
        {
            free(buffer);
            size = 0;
            buffer = NULL;
            create_css_hmap(app, &size, &buffer);
        } else memcpy(app->caches.ca, buffer + SEED_SIGNATURE_WIDTH, sizeof(CSSattribute));
    }
}

void run(App *app)
{
    resource_init(app);
    window_init(&app->window, app);
    dobj_init(app);
    vulkan_init(app);
    main_loop(app);
    clean_up(app);
}

#endif