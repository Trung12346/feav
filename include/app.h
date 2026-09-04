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
        .gpu_select_flag = -1
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
    physical_device_pick(&app->vk_process, app->gpu_select_flag);
    logical_device_create(&app->vk_process);
}
static void main_loop(GLFWwindow *window)
{
    for(;!glfwWindowShouldClose(window);)
    {
        glfwPollEvents();
    }
}
static void clean_up(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

extern void run(App *app)
{
    window_init(&app->window);
    vulkan_init(app);
    main_loop(app->window);
    clean_up(app->window);
}

#endif