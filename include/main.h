#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "debugger.h"

#define ARR_SIZE(p_size, return_size, p_arr) \
{ \
    if (p_arr == NULL) \
    { \
        printf(WARN SYS_DBG_PREFIX" Pointer to NULL\n"); \
        *p_size = 0; \
    } else \
    { \
        uintmax_t size = sizeof(p_arr) / sizeof(p_arr[0]); \
        if (size > return_size) printf(WARN SYS_DBG_PREFIX" Return size overflowed\n"); \
        *p_size = size; \
    } \
}

typedef enum
{
    false,
    true
} bool;

uint32_t unsigned_type_width_resolver(uint64_t type)
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

void shader_bin_read(const char *file, uint64_t *p_size, uint32_t *p_buffer)
{
    char path[128];
    snprintf(path, sizeof(path), "../shaders/build/%s", file);

    FILE *spv = fopen(path, "rb");
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
    
    size_t read_size = fread(p_buffer, sizeof(char), size, spv);
    if (read_size != size)
    {
        fclose(spv);
        printf(ERR SYS_DBG_PREFIX" Error occured when reading shader, exitting\n");
        exit(1);
    }

    fclose(spv);
}

uint32_t static inline hash_2_index(uint32_t hash, size_t size)
{
    return hash & (size - 1);
}

void static inline *meminject(void *main, size_t m_size, void *sub, size_t s_size, size_t injection_index, size_t *return_size)
{
    *return_size = m_size + s_size;
    void *file_builder_buffer = malloc(*return_size);
    memcpy(file_builder_buffer, main, m_size);
        
    uint64_t move_size = m_size - (injection_index + 1);
    memmove(file_builder_buffer + injection_index + s_size, file_builder_buffer + injection_index, move_size);
    memcpy(file_builder_buffer + injection_index, sub, s_size);

    return file_builder_buffer;
}

const uint8_t ascii_map[256] =
{
    ['0'] = 0U, ['1'] = 1U, ['2'] = 2U, ['3'] = 3U, ['4'] = 4U, ['5'] = 5U, ['6'] = 6U, ['7'] = 7U, ['8'] = 8U, ['9'] = 9U,
    ['A'] = 10U, ['B'] = 11U, ['C'] = 12U, ['D'] = 13U, ['E'] = 14U, ['F'] = 15U,
    ['a'] = 10U, ['b'] = 11U, ['c'] = 12U, ['d'] = 13U, ['e'] = 14U, ['f'] = 15U
};

uint64_t h_2_ull(const char *hex)
{
    uint64_t ull = 0;
    for (; *hex; hex++)
    {
        ull = ull << 4 | ascii_map[*hex];
    }
    return ull;
}
uint32_t h_2_srgb(const char *hex)
{
    return (uint32_t)
    (
        ascii_map[hex[0]] << 20
        | ascii_map[hex[1]] << 16
        | ascii_map[hex[2]] << 12
        | ascii_map[hex[3]] << 8
        | ascii_map[hex[4]] << 4
        | ascii_map[hex[5]]
    );
}
uint32_t h_2_srgba(const char *hex)
{
    return (uint32_t)
    (
        ascii_map[hex[0]] << 28
        | ascii_map[hex[1]] << 24
        | ascii_map[hex[2]] << 20
        | ascii_map[hex[3]] << 16
        | ascii_map[hex[4]] << 12
        | ascii_map[hex[5]] << 8
        | ascii_map[hex[6]] << 4
        | ascii_map[hex[7]]
    );
}

static inline void bm_bit_set(uint64_t *bm, uint64_t i)
{
    bm[i >> 6U] |= (uint64_t)1ULL << (i & 63U);
}
static inline void bm_bit_clear(uint64_t *bm, uint64_t i)
{
    bm[i >> 6U] &= ~((uint64_t)1ULL << (i & 64U));
}
static inline bool bm_bit_fetch(uint64_t *bm, uint64_t i)
{
    return bm[i >> 6U] & (uint64_t)1ULL << (i & 63U);
}
#endif