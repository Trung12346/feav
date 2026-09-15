#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdint.h>
#include "windows_utils/file.h"
#include "debugger.h"

#ifndef ARG_VAL
#define ARG_VAL char*value=argv[i+1];if(strncmp(value,"--", 2) == 0){printf(ERR SYS_DBG_PREFIX" Invalid argument for %s",argv[i]);exit(1);}
#endif

typedef enum {
    UTF8,
    UTF16,
    UTF32,
    ASCII
} Encoding;

Encoding tok_encode = ASCII;
char *tok_output_bin_filename = NULL;
bool tok_in_tag = false;

void tokenize(int argc, char **argv, char *src, uint64_t src_size, char **dst, uint64_t *dst_size)
{
    if (argc == 1);
    else
    {
        for (uint32_t i = 0U; i < argc; i++)
        {
            if (strcmp(argv[i], "--encode") == 0 && i + 1 < argc)
            {
                ARG_VAL
                if (strcmp(value, "ASCII") == 0) tok_encode = ASCII;
                else if (strcmp(value, "UTF-8") == 0) tok_encode = UTF8;
                else if (strcmp(value, "UTF-16") == 0) tok_encode = UTF16;
                else if (strcmp(value, "UTF-32") == 0) tok_encode = UTF32;
                
            } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc)
            {
                ARG_VAL
                tok_output_bin_filename = value;
            }
        }
    }
    for (;;)
    {

    }
}
#endif