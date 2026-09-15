#ifndef FILE_H
#define FILE_H

#include "debugger.h"
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

void file_read(const char *file, uint64_t *p_size, void *p_buffer)
{
    FILE *f = fopen(file, "rb");
    if (f == NULL)
    {
        printf(ERR SYS_DBG_PREFIX" Unable to read file, exitting\n");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    uint64_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    *p_size = size;

    if (p_buffer == NULL)
    {
        fclose(f);
        return;
    }
    
    size_t read_size = fread(p_buffer, sizeof(char), size, f);
    if (read_size != size)
    {
        fclose(f);
        printf(ERR SYS_DBG_PREFIX" Error occured when reading file, exitting\n");
        exit(1);
    }

    fclose(f);
}

void create_write_file(const char *file, void *src, size_t size)
{
    FILE *f = fopen(file, "wb");

    if (f == NULL)
    {
        printf(ERR SYS_DBG_PREFIX" Unable to read file, exitting\n");
        exit(1);
    }

    fwrite(src, 1, size, f);
    fclose(f);
}

#endif