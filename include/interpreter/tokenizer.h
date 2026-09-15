#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdint.h>
#include "windows_utils/file.h"
#include "debugger.h"
#include "main.h"

#ifndef ARG_VAL
#define ARG_VAL char*value=argv[i+1];if(strncmp(value,"--", 2) == 0){printf(ERR SYS_DBG_PREFIX" Invalid argument for %s",argv[i]);exit(1);}
#endif

#define BYTE 8
#define KB 1024
#define TOKEN_WIDTH 1
#define DICTIONARY_WIDTH 3

char *tok_output_bin_filename = NULL;
bool tok_in_tag = false;
bool tok_in_char = false;
bool tok_end_tag = false;
bool tok_in_str = false;

typedef enum {
    DOCTYPE,
    START_TAG,
    END_TAG,
    CHARACTER,
    TOK_EOF,
} Token;

void tokenize(int argc, char **argv, char *src, uint64_t src_size, char **dst, uint64_t *dst_size, uint8_t **dict, uint64_t *dict_size)
{
    if (argc == 1);
    else
    {
        for (uint32_t i = 0U; i < argc; i++)
        {
            if (strcmp(argv[i], "--out") == 0 && i + 1 < argc)
            {
                ARG_VAL
                tok_output_bin_filename = value;
            }
        }
    }

    /*
      1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16   <-- byte
    | b | b | b | b | b | b | b | b | b | b | b | b | b | b | b | b |
      _____________________________   _____________________________
                    |                                |
              token pointer                    character size
    */
    
    uint64_t dict_alloc_size = KB;
    *dict = malloc(dict_alloc_size);
    *dict_size = 0;
    uint64_t dst_alloc_size = KB;
    *dst = malloc(dst_alloc_size);
    *dst_size = 0;
    
    void *result;

    uint64_t strt_tag_ind = 0;
    uint64_t end_tag_ind = 0;
    uint8_t tag_size;
    char *tag_value;
    uint64_t strt_char_ind = 0;
    uint64_t end_char_ind = 0;
    uint64_t char_size;
    char *char_value;
    bool end_char_record_flag = false;
    char* insert_p;

    for (uint64_t i = 0; i < src_size; i++)
    {
        if (src[i] == '"' && tok_in_tag)
        {
            tok_in_str = !tok_in_str;
        }

        if (!tok_in_str && i + 1 < src_size && src[i] == '<' && src[i + 1] == '/')
        {
            strt_tag_ind = i + 1;
            tok_in_tag = true;
            tok_end_tag = true;
            if (!end_char_record_flag && tok_in_char) end_char_record_flag = true;
        } else if (!tok_in_str && src[i] == '<' && !tok_in_tag)
        {
            strt_tag_ind = i + 1;
            tok_in_tag = true;
            if (!end_char_record_flag && tok_in_char != 0) end_char_record_flag = true;
        } else if (!tok_in_str && src[i] == '>' && tok_in_tag && end_tag_ind == 0)
        {
            end_tag_ind = i - 1;
            tok_in_tag = false;
            
            tag_size = end_tag_ind - strt_tag_ind;

            if (!tok_end_tag)
            {
                tag_size++;
            } else {
                strt_tag_ind++;
            }
            
            tag_value = malloc(tag_size + 1);
            tag_value[tag_size] = '\0';
            memcpy(tag_value, src + strt_tag_ind, tag_size);
            printf("tag: %s\n", tag_value);

            if (*dst_size + TOKEN_WIDTH > dst_alloc_size)
            {
                dst_alloc_size += KB;
                *dst = realloc(*dst, dst_alloc_size);
            }
            if (*dict_size + 8 > dict_alloc_size)
            {
                dict_alloc_size += KB;
                *dict = realloc(*dict, dict_alloc_size);
            }

            insert_p = *dst + *dst_size;
            uint8_t token = tok_end_tag ? END_TAG : START_TAG;
            memcpy(insert_p, &token, TOKEN_WIDTH);
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 2;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 3;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 4;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 5;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 6;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 7;
            *dst_size += TOKEN_WIDTH;

            if (*dst_size + tag_size > dst_alloc_size)
            {
                dst_alloc_size += KB;
                *dst = realloc(*dst, dst_alloc_size);
            }
            if (*dict_size + 8 > dict_alloc_size)
            {
                dict_alloc_size += KB;
                *dict = realloc(*dict, dict_alloc_size);
            }
            insert_p = *dst + *dst_size;
            memcpy(insert_p, tag_value, tag_size);
            (*dict)[(*dict_size)++] = tag_size;
            *dst_size += tag_size;

            end_tag_ind = 0;
            tag_size = 0;
            strt_tag_ind = 0;
            tok_end_tag = false;
            insert_p = NULL;
        } else if (!tok_in_tag && !tok_in_char)
        {
            tok_in_char = true;
            strt_char_ind = i;
        }

        if (end_char_record_flag)
        {
            end_char_record_flag = false;
            tok_in_char = false;
            end_char_ind = i - 1;
            char_size = end_char_ind - strt_char_ind + 1;
            char_value = malloc(char_size + 1);
            char_value[char_size] = '\0';
            memcpy(char_value, src + strt_char_ind, char_size);
            printf("char: %s\n", char_value);

            if (*dst_size + TOKEN_WIDTH > dst_alloc_size)
            {
                dst_alloc_size += KB;
                *dst = realloc(*dst, dst_alloc_size);
            }
            if (*dict_size + 8 > dict_alloc_size)
            {
                dict_alloc_size += KB;
                *dict = realloc(*dict, dict_alloc_size);
            }
            insert_p = *dst + *dst_size;
            uint8_t token = CHARACTER;
            memcpy(insert_p, &token, TOKEN_WIDTH);
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 2;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 3;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 4;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 5;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 6;
            (*dict)[(*dict_size)++] = (uintptr_t)insert_p >> BYTE * 7;
            *dst_size += TOKEN_WIDTH;

            if (*dst_size + char_size > dst_alloc_size)
            {
                dst_alloc_size += KB;
                *dst = realloc(*dst, dst_alloc_size);
            }
            if (*dict_size + 8 > dict_alloc_size)
            {
                dict_alloc_size += KB;
                *dict = realloc(*dict, dict_alloc_size);
            }
            insert_p = *dst + *dst_size;
            memcpy(insert_p, char_value, char_size);
            (*dict)[(*dict_size)++] = char_size;
            (*dict)[(*dict_size)++] = char_size >> BYTE;
            (*dict)[(*dict_size)++] = char_size >> BYTE * 2;
            (*dict)[(*dict_size)++] = char_size >> BYTE * 3;
            (*dict)[(*dict_size)++] = char_size >> BYTE * 4;
            (*dict)[(*dict_size)++] = char_size >> BYTE * 5;
            (*dict)[(*dict_size)++] = char_size >> BYTE * 6;
            (*dict)[(*dict_size)++] = char_size >> BYTE * 7;
            *dst_size += char_size;
            
            strt_char_ind = 0;
            char_size = 0;
            end_char_ind = 0;
            insert_p = NULL;
        }
    }

    if (tok_output_bin_filename != NULL)
    {
        create_write_file(tok_output_bin_filename, *dst, *dst_size);
        free(tok_output_bin_filename);
    }

    free(src);
    tok_output_bin_filename = NULL;
    tok_in_tag = false;
    tok_in_char = false;
    tok_end_tag = false;
    tok_in_str = false;
}
#endif