#ifndef PARSER_H
#define PARSER_H

#include "dom/dom_entity.h"
#include "main.h"
#include "app.h"
#include "interpreter/tokenizer.h"
#include "debugger.h"

bool walk_til_char(uint64_t curr_ind, char *value, uint64_t value_size, char c, uint64_t *end_ind)
{
    for (; curr_ind < value_size; curr_ind++) if (value[curr_ind] == c) {*end_ind = curr_ind; return true;}
    return false;
}


void parse(char *src, uint8_t *dict, uint64_t dict_size, App* app)
{
    uint64_t EOF_i;
    memcpy(&EOF_i, dict + dict_size - DICT_INDEX_WIDTH, DICT_INDEX_WIDTH);
    
    if (src[EOF_i] != TOK_EOF)
    {
        printf(ERR SYS_DBG_PREFIX" Corrupted tokenized input from upstream, check tokenizer.h");
        exit(1);
    }

    uint64_t token_index;
    uint8_t token;
    uint64_t value_size;
    uint64_t value_index;
    char *value;

    uint64_t strt_value_ind;
    uint64_t end_value_ind;
    char tag_name[256];
    char attribute[256];
    char attr_value[1024 * 2];
    bool is_end = false;

    bool tag_name_passed = false;
    bool in_tag_name = false;
    bool attr_name_passed = false;
    bool in_attr_name = false;
    bool in_attr_value = false;

    //construct logical objects on memory
    for (uint64_t i = 0; i < dict_size - 1;)
    {
        value_size = 0;
        token_index = 0;
        memcpy(&token_index, dict + i, DICT_INDEX_WIDTH);
        i += DICT_INDEX_WIDTH;
        token = src[token_index];

        switch (token)
        {
            case DOCTYPE:
                i += DICT_TAG_SIZE_WIDTH;
                break;
            case START_TAG:
                value_size = dict[i];
                i += DICT_TAG_SIZE_WIDTH;
                break;
            case END_TAG:
                value_size = dict[i];
                i += DICT_TAG_SIZE_WIDTH;
                break;
            case CHARACTER:
                memcpy(&value_size, dict + i, DICT_CHAR_SIZE_WIDTH);
                i += DICT_CHAR_SIZE_WIDTH;
                break;
            default:
        }
        if (value_size != 0)
        {
            value = malloc(value_size);
            memcpy(value, src + token_index + TOKEN_WIDTH, value_size);
            printf("%s", value);

            for (uint64_t i = 0; i < value_size; i++)
            {
                if (!tag_name_passed && !in_tag_name && value[i] != ' ')
                {
                    in_tag_name = true;

                    strt_value_ind = i;
                } else if (!tag_name_passed && in_tag_name && value[i] == ' ')
                {
                    in_tag_name = false;
                    tag_name_passed = true;

                    end_value_ind = i - 1;



                    strt_value_ind = 0;
                    end_value_ind = 0;
                } else if (!tag_name_passed && in_tag_name && i == value_size - 1)
                {
                    end_value_ind = i;



                    break;
                } else if (tag_name_passed && !attr_name_passed && !in_attr_name && value[i] != ' ')
                {
                    in_attr_name = true;

                    strt_value_ind = i;
                } else if (tag_name_passed && !attr_name_passed && in_attr_name && value[i] == ' ')
                {
                    in_attr_name = false;
                    attr_name_passed = true;

                    end_value_ind = i - 1;
                    


                    strt_value_ind = 0;
                    end_value_ind = 0;
                } else if (tag_name_passed && attr_name_passed && !in_attr_name && !in_attr_value && value[i] != '=' && value[i] != ' ' && value[i] != '"')
                {
                    attr_name_passed = false;
                    in_attr_name = false;
                    i--;
                } else if (tag_name_passed && !attr_name_passed && in_attr_name && value[i] == '=')
                {
                    in_attr_name = false;
                    attr_name_passed = true;

                    end_value_ind = i - 1;



                    strt_value_ind = 0;
                    end_value_ind = 0;
                } else if (tag_name_passed && !attr_name_passed && in_attr_name && i == value_size - 1)
                {
                    end_value_ind = i;



                    break;
                } else if (tag_name_passed && attr_name_passed && !in_attr_name && !in_attr_value && value[i] == '"')
                {
                    in_attr_value = true;

                    strt_value_ind = i + 1;
                } else if (tag_name_passed && attr_name_passed && !in_attr_name && in_attr_value && value[i] == '"')
                {
                    attr_name_passed = false;
                    in_attr_value = false;

                    end_value_ind = i - 1;
                    


                    strt_value_ind = 0;
                    end_value_ind = 0;
                }
            }



            free(value);
            value = NULL;
        }
        
    }

    free(src);
    free(dict);
}


#endif