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
#include "layout_engine/resource_mgr.h"
#include "dom/dom_compute.h"
#include "hardware_utils/hash.h"
#include <stdatomic.h>

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

static void dobj_printn(DOBJHandler start, uint64_t n) {
    for (uint64_t i = 0U; i < n; i++)
    {
        DOBJPool *pool = start.pool;
        uint16_t id = start.item_index;
        printf("%-8s x=%-4.0f y=%-4.0f width=%-4.0f height=%-4.0f\n", pool->tag_names[id], pool->x[id], pool->y[id], pool->width[id], pool->height[id]);
        dobj_pool_advance(&start);
    }
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
        run(&app);
        
        // dobj_init(&app);
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



        // dobj_init(&app);
        // HeapHandler64 hh = heap_alloc(app.heap, 8, HEAP_CLASS_HEAP, &app.hflo_queue);
        // printf("%u\n", hh.a);
        // printf("%u\n", hh.s);
        // printf("%p\n", hh.heap);
        // printf("%p\n", app.heap);
        // uint64_t some_value = 0U;
        // memcpy(((Heap *)hh.heap)->mem + hh.a, &some_value, hh.s);
        // uint64_t dst;
        // memcpy(&dst, ((Heap *)hh.heap)->mem + hh.a, hh.s);
        // printf("dst: %u\n", dst);
        // heap_free(hh, HEAP_CLASS_HEAP, &app.hflo_queue);


        // dobj_init(&app);
        // printf("%p\n", app.pool);
        // DOBJPool *pool = app.pool;
        // uint64_t tree_p[4] = {0};

        // DOBJ obj = (DOBJ)
        // {
        //     .tag_name = "body",
        //     .tag_id = BODY,
        //     .x = 0,
        //     .y = 0,
        //     .width = 100,
        //     .height = 0,
        //     .p_inner.heap = NULL
        // };
        // tree_p[0] = dobj_pool_alloc(&obj, pool);
        // printf("width: %u\n", pool->width[0]);

        // DOBJ obj1 = (DOBJ){.tag_name = "div", .tag_id = DIV, .p_parent = tree_p[0], .height = 200, .p_inner.heap = NULL, .padding = padding(20, 20, 20, 20)};
        // tree_p[1] = dobj_pool_alloc(&obj1, pool);

        // DOBJ obj2 = (DOBJ){.tag_name = "div", .tag_id = DIV, .p_parent = tree_p[0], .height = 40, .p_inner.heap = NULL, .margin = margin(50, 50, 50, 50)};
        // tree_p[2] = dobj_pool_alloc(&obj2, pool);

        // const char paragraph[] = "Hello this is a filler text to make the engine new line i want this longer longer slong slong dong ding dung shit";
        // HeapHandler64 h_t = heap_alloc(app.heap, sizeof(paragraph), HEAP_CLASS_BIG_HEAP, &app.hflo_queue);
        // strcpy((char *)((BHeap *)h_t.heap)->mem + h_t.a, paragraph);

        // DOBJ obj3 = (DOBJ){.tag_name = "p", .tag_id = P, .p_parent = tree_p[0], .p_inner = h_t, .padding = padding(30, 10, 30, 10), .margin = margin(10, 20, 0, 20)};
        // tree_p[3] = dobj_pool_alloc(&obj3, pool);

        // pool->child_count[0] = 3;

        // layout_stack(pool, tree_p[0]);
        
        // dobj_printn((DOBJHandler){.pool = pool, .item_index = 0}, 4);
    }
    //terminate(&app.hflo_queue.worker);
    
    
    
    return 0;
}