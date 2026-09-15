#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void strtrim(char **src, uint64_t *size, uint64_t strt_ind, uint64_t end_ind)
{
    uint64_t end_size = *size - (end_ind + 1);
    memmove(*src + strt_ind, *src + end_ind + 1, end_size);
    *size = *size - (end_ind - strt_ind + 1);
    *src = realloc(*src, *size);
}
#endif 