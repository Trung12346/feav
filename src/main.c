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

    const char keys[][32] = {
        "margin-top",
        "margin-right",
        "margin-bottom",
        "margin-left",

        "padding-top",
        "padding-right",
        "padding-bottom",
        "padding-left",

        "border-top-width",
        "border-right-width",
        "border-bottom-width",
        "border-left-width",

        "border-top-style",
        "border-right-style",
        "border-bottom-style",
        "border-left-style",

        "border-top-color",
        "border-right-color",
        "border-bottom-color",
        "border-left-color",

        "border-top-left-radius",
        "border-top-right-radius",
        "border-bottom-right-radius",
        "border-bottom-left-radius",

        "border-collapse",
        "border-spacing",

        "border-image-source",
        "border-image-slice",
        "border-image-width",
        "border-image-outset",
        "border-image-repeat",

        "outline-color",
        "outline-style",
        "outline-width",
        "outline-offset",

        "width",
        "min-width",
        "max-width",
        "height",
        "min-height",
        "max-height",
        "box-sizing",
        "aspect-ratio",

        "position",
        "top",
        "right",
        "bottom",
        "left",
        "z-index",
        "float",
        "clear",

        "display",
        "visibility",
        "opacity",
        "overflow-x",
        "overflow-y",
        "clip",
        "clip-path",
        "object-fit",
        "object-position",

        "background-color",
        "background-image",
        "background-repeat",
        "background-position-x",
        "background-position-y",
        "background-size",
        "background-attachment",
        "background-origin",
        "background-clip",
        "background-blend-mode",

        "color",

        "font-family",
        "font-size",
        "font-style",
        "font-weight",
        "font-variant",
        "font-stretch",
        "font-size-adjust",
        "font-kerning",
        "font-feature-settings",
        "font-variation-settings",
        "line-height",

        "text-align",
        "text-align-last",
        "text-indent",
        "text-transform",
        "text-overflow",
        "text-shadow",
        "text-justify",
        "text-decoration-line",
        "text-decoration-style",
        "text-decoration-color",
        "text-decoration-thickness",
        "text-underline-offset",
        "letter-spacing",
        "word-spacing",
        "white-space",
        "word-break",
        "word-wrap",
        "overflow-wrap",
        "hyphens",
        "tab-size",
        "direction",
        "unicode-bidi",
        "writing-mode",
        "vertical-align",

        "flex-direction",
        "flex-wrap",
        "flex-grow",
        "flex-shrink",
        "flex-basis",
        "order",
        "justify-content",
        "justify-items",
        "justify-self",
        "align-content",
        "align-items",
        "align-self",
        "row-gap",
        "column-gap",

        "grid-template-columns",
        "grid-template-rows",
        "grid-template-areas",
        "grid-auto-columns",
        "grid-auto-rows",
        "grid-auto-flow",
        "grid-row-start",
        "grid-row-end",
        "grid-column-start",
        "grid-column-end",

        "column-count",
        "column-width",
        "column-fill",
        "column-span",
        "column-rule-width",
        "column-rule-style",
        "column-rule-color",

        "list-style-type",
        "list-style-position",
        "list-style-image",

        "table-layout",
        "caption-side",
        "empty-cells",

        "content",
        "quotes",
        "counter-reset",
        "counter-increment",

        "transform",
        "transform-origin",
        "transform-style",
        "perspective",
        "perspective-origin",
        "backface-visibility",
        "translate",
        "rotate",
        "scale",

        "transition-property",
        "transition-duration",
        "transition-timing-function",
        "transition-delay",

        "animation-name",
        "animation-duration",
        "animation-timing-function",
        "animation-delay",
        "animation-iteration-count",
        "animation-direction",
        "animation-fill-mode",
        "animation-play-state",

        "box-shadow",
        "filter",
        "backdrop-filter",
        "mix-blend-mode",
        "isolation",

        "mask-image",
        "mask-mode",
        "mask-repeat",
        "mask-position",
        "mask-size",
        "mask-origin",
        "mask-clip",
        "mask-composite",

        "scroll-behavior",
        "scroll-snap-type",
        "scroll-snap-align",
        "scroll-margin-top",
        "scroll-margin-right",
        "scroll-margin-bottom",
        "scroll-margin-left",
        "scroll-padding-top",
        "scroll-padding-right",
        "scroll-padding-bottom",
        "scroll-padding-left",
        "overscroll-behavior-x",
        "overscroll-behavior-y",

        "cursor",
        "pointer-events",
        "user-select",
        "resize",
        "touch-action",
        "caret-color",
        "accent-color",
        "appearance",

        "will-change",
        "contain",
        "image-rendering",
        "page-break-before",
        "page-break-after",
        "page-break-inside",
        "orphans",
        "widows"
    };

_Atomic int yes_terminate = 0;

int find_seed(const char keys[210][32], size_t n, uint64_t table_size, uint64_t min,
              uint64_t max_tries, uint64_t *out_seed)
{
    if (table_size < n) return -1;

    /* duplicate keys can never be collision-free */
    for (size_t i = 0; i < n; i++)
        for (size_t j = i + 1; j < n; j++)
            if (strcmp(keys[i], keys[j]) == 0) return -1;

    /* stamp[slot] == generation means "occupied this attempt",
     * so we never have to clear the table between seeds */
    uint64_t *stamp = calloc(table_size, sizeof *stamp);
    if (!stamp) return -1;

    size_t *lens = malloc(n * sizeof *lens);
    if (!lens) { free(stamp); return -1; }
    for (size_t i = 0; i < n; i++) lens[i] = strlen(keys[i]);

    int found = 0;
    for (uint64_t seed = min; seed < max_tries && !yes_terminate; seed++) {
        uint64_t gen = seed + 1;          /* never 0, so calloc'd state is "empty" */
        size_t i;

	if (seed % 100000 == 0) printf("at %lluth try, still not found\n");
        for (i = 0; i < n; i++) {
            uint64_t hash = fnv1a_64(keys[i], lens[i]);
	    hash = hash64(hash, seed);
            uint64_t slot = hash & (table_size - 1);
            if (stamp[slot] == gen) break; /* collision: abandon this seed */
            stamp[slot] = gen;
        }
        if (i == n) {
            *out_seed = seed;
            found = 1;
            atomic_store(&yes_terminate, 1);
            printf("terminating\n");
            break;
        }
    }

    free(lens);
    free(stamp);
    return found;
    
}



DWORD WINAPI worker0(void *arg)
{
    size_t n = sizeof keys / sizeof keys[0];
    uint64_t table_size = 2048;
    uint64_t seed;

    int r = find_seed(keys, n, table_size, 0, (uint64_t)UINT32_MAX / 4, &seed);
    if (r == 1) {
        printf("thread0: seed = %llu (table_size = %llu)\n", seed, table_size);
        for (size_t i = 0; i < n; i++) {
	    uint64_t hash = fnv1a_64(keys[i], strlen(keys[i]));
	    hash = hash64(hash, seed);
            
            printf("thread0:  %-8s -> slot %llu\n", keys[i],
                   hash % table_size);}
    } else if (r == 0) {
        printf("thread0: no seed found\n");
    } else {
        printf("thread0: invalid input or out of memory\n");
    }

    return r == 1 ? 0 : 1;
}
DWORD WINAPI worker1(void *arg)
{
    size_t n = sizeof keys / sizeof keys[0];
    uint64_t table_size = 2048;
    uint64_t seed;

    int r = find_seed(keys, n, table_size, (uint64_t)UINT32_MAX / 4, (uint64_t)UINT32_MAX / 4 * 2, &seed);
    if (r == 1) {
        printf("thread1: seed = %llu (table_size = %llu)\n", seed, table_size);
        for (size_t i = 0; i < n; i++) {
	    uint64_t hash = fnv1a_64(keys[i], strlen(keys[i]));
	    hash = hash64(hash, seed);
            
            printf("thread1:   %-8s -> slot %llu\n", keys[i],
                   hash % table_size);}
    } else if (r == 0) {
        printf("thread1: no seed found\n");
    } else {
        printf("thread1: invalid input or out of memory\n");
    }

    return r == 1 ? 0 : 1;
}
DWORD WINAPI worker2(void *arg)
{
    size_t n = sizeof keys / sizeof keys[0];
    uint64_t table_size = 2048;
    uint64_t seed;

    int r = find_seed(keys, n, table_size, (uint64_t)UINT32_MAX / 4 * 2, (uint64_t)UINT32_MAX / 4 * 3, &seed);
    if (r == 1) {
        printf("thread2: seed = %llu (table_size = %llu)\n", seed, table_size);
        for (size_t i = 0; i < n; i++) {
	    uint64_t hash = fnv1a_64(keys[i], strlen(keys[i]));
	    hash = hash64(hash, seed);
            
            printf("thread2:   %-8s -> slot %llu\n", keys[i],
                   hash % table_size);}
    } else if (r == 0) {
        printf("thread2: no seed found\n");
    } else {
        printf("thread2: invalid input or out of memory\n");
    }

    return r == 1 ? 0 : 1;
}
DWORD WINAPI worker3(void *arg)
{
    size_t n = sizeof keys / sizeof keys[0];
    uint64_t table_size = 2048;
    uint64_t seed;

    int r = find_seed(keys, n, table_size, (uint64_t)UINT32_MAX / 4 * 3, (uint64_t)UINT32_MAX, &seed);
    if (r == 1) {
        printf("thread3: seed = %llu (table_size = %llu)\n", seed, table_size);
        for (size_t i = 0; i < n; i++) {
	    uint64_t hash = fnv1a_64(keys[i], strlen(keys[i]));
	    hash = hash64(hash, seed);
            
            printf("thread3:   %-8s -> slot %llu\n", keys[i],
                   hash % table_size);}
    } else if (r == 0) {
        printf("thread3: no seed found\n");
    } else {
        printf("thread3: invalid input or out of memory\n");
    }

    return r == 1 ? 0 : 1;
}
DWORD WINAPI worker4(void *arg)
{
    size_t n = sizeof keys / sizeof keys[0];
    uint64_t table_size = 2048;
    uint64_t seed;

    int r = find_seed(keys, n, table_size, (uint64_t)UINT32_MAX, (uint64_t)UINT32_MAX / 4 * 5, &seed);
    if (r == 1) {
        printf("thread4: seed = %u (table_size = %u)\n", seed, table_size);
        for (size_t i = 0; i < n; i++) {
	    uint64_t hash = fnv1a_64(keys[i], strlen(keys[i]));
	    hash = hash64(hash, seed);
            
            printf("thread4:   %-8s -> slot %u\n", keys[i],
                   hash % table_size);}
    } else if (r == 0) {
        printf("thread4: no seed found\n");
    } else {
        printf("thread4: invalid input or out of memory\n");
    }

    return r == 1 ? 0 : 1;
}
DWORD WINAPI worker5(void *arg)
{
    size_t n = sizeof keys / sizeof keys[0];
    uint64_t table_size = 2048;
    uint64_t seed;

    int r = find_seed(keys, n, table_size, (uint64_t)UINT32_MAX / 4 * 5, (uint64_t)UINT32_MAX / 4 * 6, &seed);
    if (r == 1) {
        printf("thread5: seed = %u (table_size = %u)\n", seed, table_size);
        for (size_t i = 0; i < n; i++) {
	    uint64_t hash = fnv1a_64(keys[i], strlen(keys[i]));
	    hash = hash64(hash, seed);
            
            printf("thread5:   %-8s -> slot %u\n", keys[i],
                   hash % table_size);}
    } else if (r == 0) {
        printf("thread5: no seed found\n");
    } else {
        printf("thread5: invalid input or out of memory\n");
    }

    return r == 1 ? 0 : 1;
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

        #if defined(__AES__)
            printf("using x84-64\n");
        #endif

        WorkersProperties wp;
        WorkersGetProperties(&wp);
        printf("logical core:");
        for (uint8_t i = 0; i < wp.logical_core_count; i++)
        {
            printf("thread: %u @ core: %u\n",
                wp.physical_device_core_n_thread_relationship[i].thread,
                wp.physical_device_core_n_thread_relationship[i].core
            );
        }

        DWORD_PTR core0 = (DWORD_PTR)1;
        DWORD_PTR core1 = (DWORD_PTR)1 << 2;
        DWORD_PTR core2 = (DWORD_PTR)1 << 4;
        DWORD_PTR core3 = (DWORD_PTR)1 << 6;
        DWORD_PTR core4 = (DWORD_PTR)1 << 8;
        DWORD_PTR core5 = (DWORD_PTR)1 << 10;

        HANDLE thread0 = CreateThread(
                NULL, 0, worker0, NULL, 0, NULL
        );
        HANDLE thread1 = CreateThread(
                NULL, 0, worker1, NULL, 0, NULL
        );
        HANDLE thread2 = CreateThread(
                NULL, 0, worker2, NULL, 0, NULL
        );
        HANDLE thread3 = CreateThread(
                NULL, 0, worker3, NULL, 0, NULL
        );
        HANDLE thread4 = CreateThread(
                NULL, 0, worker4, NULL, 0, NULL
        );
        HANDLE thread5 = CreateThread(
                NULL, 0, worker5, NULL, 0, NULL
        );
        SetThreadAffinityMask(thread0, core0);
        SetThreadAffinityMask(thread1, core1);
        SetThreadAffinityMask(thread2, core2);
        SetThreadAffinityMask(thread3, core3);
        SetThreadAffinityMask(thread4, core4);
        SetThreadAffinityMask(thread5, core5);

        HANDLE threadHandles[6] = { thread0, thread1, thread2, thread3, thread4, thread5 };
        WaitForMultipleObjects(6, threadHandles, TRUE, INFINITE);



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