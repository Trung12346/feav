#ifndef PARSER_H
#define PARSER_H

#include "dom/dom_entity.h"
#include "main.h"
#include "app.h"
#include "interpreter/tokenizer.h"
#include "debugger.h"
#include "ryu/ryu_parse.h"

#define BUILD_DOM_OP_MODE_MASK 0b11110000
#define HEAP_CSS_ATTRIBUTE_STRIDE 2
#define HEAP_CSS_VALUE_STRIDE 4
#define VALUE_BUFFER_SIZE 1024 * 2

bool is_separator(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0');
}

typedef enum {
    CSS_BEFORE_PROPERTY,
    CSS_IN_PROPERTY,
    CSS_AFTER_PROPERTY,
    CSS_BEFORE_VALUE,
    CSS_IN_VALUE,
    CSS_AFTER_VALUE
} CSS_PARSER_STATE;

HeapHandler16 css_parse(const char *css, App *app)
{
    const char *p_strt;
    uint64_t len = 0;
    const char *p = css;
    CSS_PARSER_STATE state = CSS_BEFORE_PROPERTY;
    CSSattribute *cache = app->caches.ca;

    uint64_t buffer_alloc_size = 1024;
    uint64_t buffer_size = 0;
    uint8_t *buffer = malloc(buffer_alloc_size); 

    uint16_t css_id;
    uint32_t value;

    for (; *p; p++)
    {
        switch (state)
        {
            case CSS_BEFORE_PROPERTY:
                if (!is_separator(*p))
                {
                    p_strt = p;
                    state = CSS_IN_PROPERTY;
                }
                break;
            case CSS_IN_PROPERTY:
                if (*p == ':')
                {
                    state = CSS_BEFORE_VALUE;
                    len = p - p_strt;
                    css_id = css_attrn_hmap_id_get(p_strt, len);
                    if (buffer_size + HEAP_CSS_ATTRIBUTE_STRIDE > buffer_alloc_size)
                    {
                        buffer_alloc_size += 1024;
                        buffer = realloc(buffer, buffer_alloc_size);
                    }
                    memcpy(buffer + buffer_size, &css_id, HEAP_CSS_ATTRIBUTE_STRIDE);
                    buffer_size += HEAP_CSS_ATTRIBUTE_STRIDE;
                } else if (is_separator(*p))
                {
                    state = CSS_AFTER_PROPERTY;
                    len = p - p_strt;
                    css_id = css_attrn_hmap_id_get(p_strt, len);
                    if (buffer_size + HEAP_CSS_ATTRIBUTE_STRIDE + HEAP_CSS_VALUE_STRIDE > buffer_alloc_size)
                    {
                        buffer_alloc_size += 1024;
                        buffer = realloc(buffer, buffer_alloc_size);
                    }
                    memcpy(buffer + buffer_size, &css_id, HEAP_CSS_ATTRIBUTE_STRIDE);
                    buffer_size += HEAP_CSS_ATTRIBUTE_STRIDE;
                }
                break;
            case CSS_AFTER_PROPERTY:
                if (*p == ':') state = CSS_BEFORE_VALUE;
                break;
            case CSS_BEFORE_VALUE:
                if (!is_separator(*p))
                {
                    state = CSS_IN_VALUE;
                    p_strt = p;
                }
                break;
            case CSS_IN_VALUE:
                bool pass = false;
                if (is_separator(*p))
                {
                    state = CSS_AFTER_VALUE;
                    pass = true;
                } else if (*p == ';')
                {
                    state = CSS_BEFORE_PROPERTY;
                    pass = true;
                }

                if (pass)
                {
                    len = p - p_strt;
                    if (css_id == cache->CSS_BACKGROUND_COLOR)
                    {
                        switch (len)
                        {
                            case 7: //+1 accounting for the hex designator #
                                value = h_2_srgb(p_strt + 1);
                                break;
                            case 9:
                                value = h_2_srgba(p_strt + 1);
                                break;
                            default:
                                //handle exception
                        }
                    } //handle more special value
                    else
                    {
                        float f;
                        s2f_n(p_strt, len, &f);
                        memcpy(&value, &f, HEAP_CSS_VALUE_STRIDE);
                    }
                    memcpy(buffer + buffer_size, &value, HEAP_CSS_VALUE_STRIDE);
                    buffer_size += HEAP_CSS_VALUE_STRIDE;
                }
                break;
            case CSS_AFTER_VALUE:
                if (*p == ';') state = CSS_BEFORE_PROPERTY;
                break;
        }
    }

    HeapHandler64 hh = heap_alloc(app->heap, buffer_size, HEAP_CLASS_HEAP, &app->hflo_queue);
    memcpy(((Heap *)hh.heap)->mem + hh.a, buffer, hh.s);
    free(buffer);

    return (HeapHandler16){.heap = hh.heap, .a = hh.a, .s = hh.s};
}

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
    const char style[VALUE_BUFFER_SIZE];
    uint16_t style_size;
} BuildDOBJinfo;

DOBJHandler build_dom(BuildDOBJinfo *build_info)
{
    if (build_info->option & UPDATE)
    {
        DOBJHandler obj_h = pool_search(build_info->ref_pool, build_info->glb_id);
    } else if (build_info->option & CREATE)
    {
        

    }
}
typedef struct {
    DOBJPool *ref_pool;
    const char *tag;
    uint8_t t_size;
    DOBJHandler h_parent;
} DOBJcreateInfo;

DOBJHandler create_dobj(DOBJcreateInfo *info)
{
    DOBJ new_obj = (DOBJ){};

    strcpy(new_obj.tag_name, info->tag);
    new_obj.tag_id = html_tag_hmap_id_get(info->tag);

    DOBJHandler h_obj = dobj_pool_alloc(&new_obj, info->ref_pool);
    DOBJPool *p = h_obj.pool;
    uint64_t id = h_obj.item_index;

    p->tag_ids[id] = html_tag_hmap_id_get(info->tag);
    memcpy(&p->tag_names[id], info->tag, info->t_size);
    p->DOBJHandler_pool_parent[id] = info->h_parent.pool;
    p->DOBJHandler_item_index_parent[id] = info->h_parent.item_index;

    return h_obj;
}

//considering removing support for css inline attribute like bgcolor, hidden, align, etc.
void parse(char *src, uint8_t *dict, uint64_t dict_size, App* app)
{
    DOBJPool *pool = app->pool;

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
    char attr_value[VALUE_BUFFER_SIZE];
    uint8_t tag_size;
    uint8_t attr_name_size;
    uint16_t attr_value_size;

    PARSER_STATE state = BEFORE_TAG;
    bool enclosed_value = false;
    bool value_transit = false;

    DOBJHandler o;
    DOBJHandler p = (DOBJHandler){.pool = NULL};

    // uint16_t attr_alloc_size = 128;
    // uint16_t attr_count = 0;
    // CSSattribute *attrs = malloc(attr_alloc_size);
    // float *attr_values = malloc(attr_alloc_size);

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

            if (token == START_TAG)
            {
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
                                tag_name[tag_size++] = '\0';
                                printf("tag: %s\n", tag_name);

                                DOBJcreateInfo create_info = (DOBJcreateInfo)
                                {
                                    .tag = tag_name,
                                    .t_size = tag_size,
                                    .ref_pool = pool,
                                    .h_parent = p
                                };
                                o = create_dobj(&create_info);
                                p = o;
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
                                attribute[attr_name_size++] = '\0';

                                printf("attribute name: %s\n", attribute);
                            } else if (is_separator(value[i])) //register attribute with unset value
                            {
                                end_value_ind = i - 1;
                                state = AFTER_ATTR_NAME;

                                attr_name_size = end_value_ind - strt_value_ind + 1;
                                memcpy(attribute, value + strt_value_ind, attr_name_size);
                                attribute[attr_name_size++] = '\0';
                                printf("attribute name: %s\n", attribute);
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
                                //process attribute whose value is not set
                                if (strcmp(attribute, "hidden"))
                                {

                                } else if (strcmp(attribute, "disabled"))
                                {

                                } else if (strcmp(attribute, "readonly"))
                                {
                                    
                                } else if (strcmp(attribute, "selected"))
                                {

                                } else if (strcmp(attribute, "checked"))
                                {

                                }
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
                            if (value[i] == '\'' || value[i] == '"' || (is_separator(value[i]) && !enclosed_value))
                            {
                                end_value_ind = i - 1;
                                enclosed_value = false;
                                state = BEFORE_ATTR_NAME;

                                attr_value_size = end_value_ind - strt_value_ind + 1;
                                memcpy(attr_value, value + strt_value_ind, attr_value_size);
                                attr_value[attr_value_size++] = '\0';
                                printf("attribute value: %s\n", attr_value);

                                //process attribute with value
                                if (strcmp(tag_name, "style"))
                                {
                                    // memcpy(build_info.style, attr_value, attr_value_size);
                                    // build_info.style_size = attr_value_size;
                                }
                            }
                            break;
                    }
                }
                
                state = BEFORE_TAG;
                enclosed_value = false;
                value_transit = false;
            } else if (token == END_TAG)
            {
                p = dobj_get_h_parent(o);
            }


            free(value);
            value = NULL;
            
        }
        
    }

    // free(attrs);
    // free(attr_values);
    free(src);
    free(dict);
}

#endif