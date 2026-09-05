#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"
#include "vk_process.h"
// #include "cglm/"
#include "app.h"
#include "main.h"

int main(int argc, char **argv)
{
    HWND console = GetConsoleWindow();

    bool is_special_op = false;
    App app = app_no_args_construct();

    if (argc == 1);
    else
    {
        for (uint32_t i = 0U; i < argc; i++)
        {
            //main operation
            if (strcmp(argv[i], "--gpu-select") == 0 && i + 1 < argc)
            {
                char *ptr_end;
                app.gpu_select_flag = strtol(argv[i + 1U], &ptr_end, 10);
                if (*ptr_end != '\0')
                {
                    printf(ERR SYS_DBG_PREFIX" Invalid argument for gpu-select");
                    exit(1);
                }
            } else if (strcmp(argv[i], "--no-console") == 0)
            {
                ShowWindow(console, SW_HIDE);
            }

            //special operation
            else if (strcmp(argv[i], "-rd") == 0)
            {
                is_special_op = true;
                instance_create(&app.vk_process);
                messenger_debug_setup(&app.vk_process);
                physical_device_pick(&app.vk_process, -1);
            }
        }
    }
    
    if (!is_special_op)
    {
        run(&app);
    }
    
    
    
    return 0;
}