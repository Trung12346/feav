#ifndef PARSER_H
#define PARSER_H

#include "dom/dom_entity.h"
#include "main.h"
#include "app.h"
#include "interpreter/tokenizer.h"
#include "debugger.h"

void parse(char *src, uint8_t *dict, uint64_t dict_size, App* app)
{
    uint64_t EOF_i;
    memcpy(&EOF_i, dict + dict_size - DICT_INDEX_WIDTH, DICT_INDEX_WIDTH);
    
    if (src[EOF_i] != TOK_EOF)
    {
        printf(ERR SYS_DBG_PREFIX" Corrupted tokenized input from upstream, check tokenizer.h");
        exit(1);
    }

    uint64_t index;
    uint64_t value_index;
    uint64_t size;
    char *value;

    printf("%u", dict_size);
    //construct logical objects on memory
    for (uint64_t i = 0; i < dict_size - 1;)
    {
        memcpy(&index, dict + i, DICT_INDEX_WIDTH);
        i += DICT_INDEX_WIDTH;

        uint8_t token = src[index];
        switch (token)
        {
            case START_TAG:
                memcpy(&size, dict + i, DICT_TAG_SIZE_WIDTH);
                
                value = malloc(size);
                memcpy(value, src + index, size);
                printf("%s", value);

                free(value);
                value = NULL;
                i += DICT_TAG_SIZE_WIDTH;
                break;
            case CHARACTER:
                memcpy(&size, dict + i, DICT_CHAR_SIZE_WIDTH);
                
                value = malloc(size);
                memcpy(value, src + index, size);
                printf("%s", value);

                free(value);
                value = NULL;
                i += DICT_CHAR_SIZE_WIDTH;
                break;
            case END_TAG:
                memcpy(&size, dict + i, DICT_TAG_SIZE_WIDTH);
                
                value = malloc(size);
                memcpy(value, src + index, size);
                printf("%s", value);

                free(value);
                value = NULL;
                i += DICT_TAG_SIZE_WIDTH;
                break;
            default:
        }
    }

    free(src);
    free(dict);
}


#endif