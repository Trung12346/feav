#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <stdio.h>

#define ARR_SIZE(p_size, return_size, p_arr) \
{ \
    if (p_arr == NULL) \
    { \
        printf(WARN SYS_DBG_PREFIX" Pointer to NULL\n"); \
        *p_size = 0; \
    } else \
    { \
        uintmax_t size = sizeof(p_arr) / sizeof(p_arr[0]); \
        if (size >> unsigned_type_width_resolver(return_size)) printf(WARN SYS_DBG_PREFIX" Return size overflowed\n"); \
        *p_size = size; \
    } \
}

typedef enum
{
    false,
    true
} bool;

extern uint32_t unsigned_type_width_resolver(uintmax_t type)
{
    switch (type)
    {
        case UINT8_MAX: return 8U;
        case UINT16_MAX: return 16U;
        case UINT32_MAX: return 32U;
        case UINT64_MAX: return 64U;
        default: printf(WARN SYS_DBG_PREFIX" Unknown type to be resolved\n"); return 0;
    }
}

extern void shader_bin_read(const char *file, uint64_t *p_size, uint32_t *p_buffer)
{
    char path[128];
    snprintf(path, sizeof(path), "../shaders/build/%s", file);

    FILE *spv = fopen(path, "r");
    if (spv == NULL)
    {
        printf(ERR SYS_DBG_PREFIX" Unable to read shader, exitting\n");
        exit(1);
    }
    fseek(spv, 0, SEEK_END);
    uint64_t size = ftell(spv);
    fseek(spv, 0, SEEK_SET);

    *p_size = size;

    if (p_buffer == NULL) 
    {
        fclose(spv);
        return;
    }
    
    fread(p_buffer, sizeof(char), size, spv);
    fclose(spv);
}
// extern uint64_t arr_size(void *p_arr, uint64_t return_size)
// {
//     if (p_arr == NULL)
//     {
//         printf(WARN SYS_DBG_PREFIX" Potential alrithmetic error\n");
//         return 0;
//     }
//     uint64_t size = sizeof(p_arr) / sizeof(p_arr[0]);

//     if (size >> unsigned_type_width_resolver(return_size))
//     {
//         printf(WARN SYS_DBG_PREFIX" Return size overflowed\n");
//     }
//     return size;
// }

#endif