#ifndef APP_H
#define APP_H

#include "GLFW/glfw3.h"
#include "vk_process.h"

#define VIEW_PORT_WIDTH 1000
#define VIEW_PORT_HEIGHT 760
typedef struct
{
    GLFWwindow *window;
    VkProcess vk_process;
    int gpu_select_flag;
} App;
extern App app_no_args_construct()
{
    return (App)
    {
        .gpu_select_flag = -1,
        .vk_process = vk_process_no_args_construct()
    };
}
static void window_init(GLFWwindow **window)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    *window = glfwCreateWindow(VIEW_PORT_WIDTH, VIEW_PORT_HEIGHT, "Vulkan", NULL, NULL);
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
    command_buffer_create(&app->vk_process);
    sync_object_create(&app->vk_process);
}
static void main_loop(App *app)
{
    for(;!glfwWindowShouldClose(app->window);)
    {
        glfwPollEvents();
        frame_draw(&app->vk_process);
    }
    vkDeviceWaitIdle(*app->vk_process.logical_device);
}
static void vulkan_destroy(VkProcess *process)
{
    VkDevice *device = process->logical_device;
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
    
}
static void clean_up(App *app)
{
    vulkan_destroy(&app->vk_process);
    glfwDestroyWindow(app->window);
    glfwTerminate();
}

extern void run(App *app)
{
    window_init(&app->window);
    vulkan_init(app);
    main_loop(app);
    clean_up(app);
}

#endif