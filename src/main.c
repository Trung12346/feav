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
#include "interpreter/preprocessor.h"
#include "interpreter/tokenizer.h"
#include "interpreter/parser.h"

#ifndef ARG_VAL
#define ARG_VAL char*value=argv[i+1];if(strncmp(value,"--", 2) == 0){printf(ERR SYS_DBG_PREFIX" Invalid argument for %s",argv[i]);exit(1);}
#endif

uint8_t preproc_argc = 0;
char *preproc_argv[2];

uint8_t tok_argc = 0;
char *tok_argv[2];

void print_array(const uint8_t *array, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        printf("0x%02X ", array[i]);
    }

    printf("\n");
}

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
            } else if (strcmp(argv[i], "--preproc-out") == 0 && i + 1 < argc)
            {
                ARG_VAL
                preproc_argc += 2;
                preproc_argv[preproc_argc - 2] = "--out";
                preproc_argv[preproc_argc - 1] = value;

                printf("%s %s\n", preproc_argv[preproc_argc - 2], preproc_argv[preproc_argc - 1]);
            } else if (strcmp(argv[i], "--tok-out") == 0 && i + 1 < argc)
            {
                ARG_VAL
                tok_argc += 2;
                tok_argv[tok_argc - 2] = "--out";
                tok_argv[tok_argc - 1] = value;

                printf("%s %s\n", tok_argv[tok_argc - 2], tok_argv[tok_argc - 1]);
            }

            //special operation
            else if (strcmp(argv[i], "-rd") == 0) //read physical devices
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
        // run(&app);
        
        // char *preproced;
        // uint64_t size;
        // preprocess(preproc_argc, preproc_argv, &preproced, &size);
        // printf("size  %u\n", size);

        // char *dst;
        // uint64_t dst_size;
        // uint8_t *dict;
        // uint64_t dict_size;
        // tokenize(tok_argc, tok_argv, preproced, size, &dst, &dst_size, &dict, &dict_size);
        
        // parse(dst, dict, dict_size, &app);
        dobj_init(&app);
    }
    
    
    
    return 0;
}