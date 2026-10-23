#ifndef DOM_COMPUTE_H
#define DOM_COMPUTE_H

#include "dom/specs.h"
#include "dom/dom_entity.h"
#include "app.h"
#include "math.h"

#define CHARACTER_WIDTH 8.f
#define CHARACTER_HEIGHT 20.f

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

float layout_content_width(DOBJHandler h)
{
    return h.pool->width[h.item_index] - h.pool->padding[h.item_index].left - h.pool->padding[h.item_index].right;
}

float layout_text_height(const char *p_t, float width_available)
{
    uint16_t line_count = 1;
    float line_width_current = 0;
    const char *p_cursor = p_t;

    while (*p_cursor)
    {
        const char *p_word_start = p_cursor;
        while (*p_cursor && *p_cursor != ' ') p_cursor++;
        const char *p_word_end = p_cursor;

        float word_width = (p_word_end - p_word_start) * CHARACTER_WIDTH;
        if (line_width_current == 0)
        {
            line_width_current = word_width;
        } else if (line_width_current + word_width + CHARACTER_WIDTH <= width_available)
        {
            line_width_current += word_width + CHARACTER_WIDTH;
        } else
        {
            line_count++;
            line_width_current = word_width;
        }
        while (*p_cursor == ' ') p_cursor++;
    }

    return line_count * CHARACTER_HEIGHT;
}

void layout_stack(DOBJPool *pool, uint64_t p_parent)
{
    DOBJHandler h_parent = pool_search(pool, p_parent);

    float content_width = layout_content_width(h_parent);
    float y_cursor = h_parent.pool->padding[h_parent.item_index].top;
    uint16_t child_count = h_parent.pool->child_count[h_parent.item_index];
    float prev_element_margin = 0;

    DOBJHandler h_child = h_parent;
    pool = h_child.pool;
    dobj_pool_advance(&h_child);
    DOBJHandler h_origin = h_child;

    for (uint16_t i = 0U; i < child_count; i++)
    {
        uint64_t id = h_child.item_index;

        if (pool->position[id] == CSS_ATTRIBUTE_POSITION_ABSOLUTE) continue;

        pool->x[id] = h_parent.pool->padding[h_parent.item_index].left + pool->margin[id].left;
        pool->y[id] = y_cursor + MAX(h_child.pool->margin[id].top, prev_element_margin);
        pool->width[id] = content_width - pool->margin[id].left - pool->margin[id].right;

        BHeap *h = pool->HeapHandler64_heap_h_inner[id];
        if (h != NULL)
        {
            uint64_t a = pool->HeapHandler64_a_h_inner[id];
            const char *p_t = (const char *)h->mem + a;
            float text_height = layout_text_height(p_t, layout_content_width(h_child));
            printf("textheight %f\n", text_height);
            printf("avaiwidth %f\n", layout_content_width(h_child));
            pool->height[id] = pool->padding[id].top + text_height + pool->padding[id].bottom;
        }

        y_cursor = h_child.pool->y[h_child.item_index] + h_child.pool->height[h_child.item_index];
        prev_element_margin = h_child.pool->margin[h_child.item_index].bottom;

        dobj_pool_advance(&h_child);
        pool = h_child.pool;
    }

    if (!(h_parent.pool->is_set_g1[h_parent.item_index] & DOBJ_PROPERTIES_IS_SET_GROUP1_HEIGHT_BIT)) h_parent.pool->height[h_parent.item_index] = y_cursor + h_parent.pool->padding[h_parent.item_index].bottom + prev_element_margin;
    
    h_child = h_origin;
    for (uint16_t i = 0U; i < child_count; i++)
    {
        uint64_t id = h_child.item_index;
        if (pool->position[id] == CSS_ATTRIBUTE_POSITION_ABSOLUTE)
        {

        }
        dobj_pool_advance(&h_child);
        pool = h_child.pool;
    }
}



#endif