#ifndef CACHE_H
#define CACHE_H

#include "windows_utils/file.h"
#include "debugger.h"
#include "dom/specs.h"
#include "main.h"

typedef struct {
    CSSattribute *ca; //"runtime-enum", enum configuration is dependent on hashing algorithm
    HTMLtag *ha; //this was meant to be named ht but guess i will refactor it later
    uint64_t hsctbm[HTML_TAG_HMAP_SLOT_COUNT / QWORD_WIDTH]; //html self-close tags bitmap
} AppCaches;



void hsctbm_init(AppCaches *cache)
{
    HTMLtag *ht = cache->ha;
    bm_bit_set(cache->hsctbm, ht->HTML_LINK);
    bm_bit_set(cache->hsctbm, ht->HTML_META);
    bm_bit_set(cache->hsctbm, ht->HTML_BR);
    bm_bit_set(cache->hsctbm, ht->HTML_HR);
    bm_bit_set(cache->hsctbm, ht->HTML_IMG);
    bm_bit_set(cache->hsctbm, ht->HTML_INPUT);
    bm_bit_set(cache->hsctbm, ht->HTML_SOURCE);
}

void create_html_hmap(AppCaches *cache, size_t *size, void **buffer)
{
    file_read("../assets/html-tags.bin", size, NULL);
    if (*size == 0)
    {
        printf(ERR SYS_DBG_PREFIX" Missing resource \"html-tags.bin\"");
        exit(1);
    }
    *buffer = malloc(*size);
    file_read("../assets/html-tags.bin", size, *buffer);

    const char *cursor_resrc_p = *buffer;
    uint16_t *cursor_struct_p = (uint16_t *)cache->ha;
    for (uint8_t i = 0U; i < HTML_TAG_COUNT; i++)
    {
        *cursor_struct_p = html_tag_hmap_id_get(cursor_resrc_p);

        cursor_struct_p++;
        cursor_resrc_p += HTML_TAG_RESRC_STRIDE;
    }
    void *write_buffer = malloc(sizeof(HTMLtag) + SEED_SIGNATURE_STRIDE);
    *(uint64_t *)write_buffer = SEED_HTML_TAG;
    memcpy(write_buffer + SEED_SIGNATURE_STRIDE, cache->ha, sizeof(HTMLtag));
    create_write_file("../assets/html-tags-enum.bin", write_buffer, sizeof(HTMLtag) + SEED_SIGNATURE_STRIDE);
}
void create_css_hmap(AppCaches *cache, size_t *size, void **buffer)
{
    file_read("../assets/css-attributes.bin", size, NULL);
    if (*size == 0)
    {
        printf(ERR SYS_DBG_PREFIX" Missing resource \"css-attributes.bin\"");
        exit(1);
    }
    *buffer = malloc(*size);
    file_read("../assets/css-attributes.bin", size, *buffer);

    const char *cursor_resrc_p = *buffer;
    uint16_t *cursor_struct_p = (uint16_t *)cache->ca;
    for (uint8_t i = 0U; i < CSS_ATTRIBUTE_COUNT; i++)
    {
        *cursor_struct_p = css_attr_hmap_id_get(cursor_resrc_p);

        cursor_struct_p++;
        cursor_resrc_p += CSS_ATTRIBUTE_RESRC_STRIDE;
    }
    void *write_buffer = malloc(sizeof(CSSattribute) + SEED_SIGNATURE_STRIDE);
    *(uint64_t *)write_buffer = SEED_CSS_ATTRIBUTE;
    memcpy(write_buffer + SEED_SIGNATURE_STRIDE, cache->ca, sizeof(CSSattribute));
    create_write_file("../assets/css-attributes-enum.bin", write_buffer, sizeof(CSSattribute) + SEED_SIGNATURE_STRIDE);
}
void resource_init(AppCaches *cache)
{
    cache->ha = calloc(1, sizeof(HTMLtag));

    size_t size = 0;
    void *buffer;

    file_read("../assets/html-tags-enum.bin", &size, NULL);

    if (size == 0) create_html_hmap(cache, &size, &buffer);
    else
    {
        buffer = malloc(size);
        file_read("../assets/html-tags-enum.bin", &size, buffer);
        if (*(uint64_t *)buffer != SEED_HTML_TAG || size != (sizeof(HTMLtag) + SEED_SIGNATURE_STRIDE))
        {
            free(buffer);
            size = 0;
            buffer = NULL;
            create_html_hmap(cache, &size, &buffer);
        } else memcpy(cache->ha, buffer + SEED_SIGNATURE_STRIDE, sizeof(HTMLtag));
    }

    free(buffer);
    buffer = NULL;
    size = 0;

    cache->ca = calloc(1, sizeof(CSSattribute));
    file_read("../assets/css-attributes-enum.bin", &size, NULL);

    if (size == 0) create_css_hmap(cache, &size, &buffer);
    else
    {
        buffer = malloc(size);
        file_read("../assets/css-attributes-enum.bin", &size, buffer);
        if (*(uint64_t *)buffer != SEED_CSS_ATTRIBUTE || size != (sizeof(CSSattribute) + SEED_SIGNATURE_STRIDE))
        {
            free(buffer);
            size = 0;
            buffer = NULL;
            create_css_hmap(cache, &size, &buffer);
        } else memcpy(cache->ca, buffer + SEED_SIGNATURE_STRIDE, sizeof(CSSattribute));
    }


    hsctbm_init(cache);
}

#endif