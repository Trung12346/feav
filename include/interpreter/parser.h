#ifndef PARSER_H
#define PARSER_H

#include "dom/dom_entity.h"
#include "main.h"
#include "app.h"
#include "interpreter/tokenizer.h"
#include "debugger.h"

#define BUILD_DOM_OP_MODE_MASK 0b11110000

typedef enum {
    BEFORE_TAG,
    IN_TAG,
    BEFORE_ATTR_NAME,
    IN_ATTR_NAME,
    AFTER_ATTR_NAME,
    BEFORE_ATTR_VALUE,
    IN_ATTR_VALUE,
    END
} PARSER_STATE;

typedef uint8_t BuildDOBJoption; enum {
    TAG_NAME   = 0b00000001,
    ATTRIBUTE  = 0b00000010,
    ATTR_VALUE = 0b00000100,

    CREATE     = 0b00010000,
    UPDATE     = 0b00100000
};
/*
      1   2   3   4   5   6   7   8   <-- bit
    | b | b | b | b | b | b | b | b |
      _____________   _____________
            |               |
        type flags    operation mode
*/
typedef struct {
    BuildDOBJoption option;
    DOBJPool *ref_pool;
    uint64_t glb_id;
    const char *tag;
    uint8_t t_size;
    const char *attr_n;
    uint8_t an_size;
    const char *attr_v;
    uint16_t av_size;
} BuildDOBJinfo;

DOBJHandler build_dom(BuildDOBJinfo *build_info)
{
    if (build_info->option & UPDATE)
    {
        DOBJHandler obj_h = pool_search(build_info->ref_pool, build_info->glb_id);
    } else if (build_info->option & CREATE)
    {
        DOBJ new_obj = (DOBJ){};
        if (build_info->option & TAG_NAME)
        {
            strcpy(new_obj.tag_name, build_info->tag);
            new_obj.tag_id = tag_name_resolver(build_info->tag);

            build_info->option &= ~TAG_NAME;
        }
        DOBJHandler obj_h = dobj_pool_alloc(&new_obj, build_info->ref_pool);
        DOBJPool *p = obj_h.pool;
        uint64_t id = obj_h.item_index;

        p->tag_ids[id] = html_tag_hmap_id_get(build_info->tag);
        strncpy(&p->tag_names[id], build_info->tag, build_info->t_size);

    }
}

bool is_separator(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0');
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
    uint8_t tag_size;
    uint8_t attr_name_size;
    uint16_t attr_value_size;

    PARSER_STATE state = BEFORE_TAG;
    bool enclosed_value = false;
    bool value_transit = false;

    uint16_t attr_alloc_size = 128;
    uint16_t attr_count = 0;
    CSSattribute *attrs = malloc(attr_alloc_size);
    float *attr_values = malloc(attr_alloc_size);

    BuildDOBJinfo build_info = (BuildDOBJinfo){};

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
            //printf("%s", value);

            for (uint64_t i = 0; i < value_size; i++)
            {
                switch (state)
                {
                    case BEFORE_TAG:
                        if (!is_separator(value[i]))
                        {
                            strt_value_ind = i;
                            state = IN_TAG;
                        }
                        break;
                    case IN_TAG:
                        if (is_separator(value[i]))
                        {
                            end_value_ind = i - 1;
                            state = BEFORE_ATTR_NAME;

                            tag_size = end_value_ind - strt_value_ind + 1;
                            memcpy(tag_name, value + strt_value_ind, tag_size);
                            tag_name[strt_value_ind + tag_size++] = '\0';
                        }
                        break;
                    case BEFORE_ATTR_NAME:
                        if (!is_separator(value[i]))
                        {
                            strt_value_ind = i;
                            state = IN_ATTR_NAME;
                        }
                        break;
                    case IN_ATTR_NAME:
                        if (value[i] == '=') //transitioning from reading attribute name to attribute value
                        {
                            end_value_ind = i - 1;
                            state = BEFORE_ATTR_VALUE;

                            attr_name_size = end_value_ind - strt_value_ind + 1;
                            memcpy(attribute, value + strt_value_ind, attr_name_size);
                            attribute[strt_value_ind + attr_name_size++] = '\0';
                        } else if (is_separator(value[i])) //register attribute with unset value
                        {
                            end_value_ind = i - 1;
                            state = AFTER_ATTR_NAME;

                            attr_name_size = end_value_ind - strt_value_ind + 1;
                            memcpy(attribute, value + strt_value_ind, attr_name_size);
                            attribute[strt_value_ind + attr_name_size++] = '\0';

                            //attrs[attr_count] = 
                        }
                        break;
                    case AFTER_ATTR_NAME:
                        if (value[i] == '=')
                        {
                            state = BEFORE_ATTR_VALUE;
                        } else if (!is_separator(value[i]))
                        {
                            state = IN_ATTR_NAME;
                            strt_value_ind = i;
                        }
                        break;
                    case BEFORE_ATTR_VALUE:
                        if (value[i] == '\'' || value[i] == '"')
                        {
                            enclosed_value = true;
                            strt_value_ind = i + 1;
                            state = IN_ATTR_VALUE;
                        } else if (!is_separator(value[i]))
                        {
                            strt_value_ind = i;
                            state = IN_ATTR_VALUE;
                        }
                        break;
                    case IN_ATTR_VALUE:
                        if (value[i] == '\'' || value[i] == '"' || is_separator(value[i]))
                        {
                            end_value_ind = i - 1;
                            state = BEFORE_ATTR_NAME;

                            attr_value_size = end_value_ind - strt_value_ind + 1;
                            memcpy(attr_value, value + strt_value_ind, attr_value_size);
                            attr_value[strt_value_ind + attr_value_size++] = '\0';
                        }
                        break;
                }
            }

            free(value);
            value = NULL;
        }
        
    }

    free(attrs);
    free(attr_values);
    free(src);
    free(dict);
}

#endif