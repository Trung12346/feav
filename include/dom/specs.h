#ifndef DOM_SPECS_H
#define DOM_SPECS_H

#include <stdint.h>
#include <stdatomic.h>
#include <windows.h>
#include "hardware_utils/hash.h"
#include "main.h"

#define QWORD_SCAN_IS_OCCUPIED_MASK 0b0000000100000001000000010000000100000001000000010000000100000001ULL
#define DWORD_SCAN_IS_OCCUPIED_MASK 0b00000001000000010000000100000001UL
#define HALFWORD_SCAN_IS_OCCUPIED_MASK 0b00000001U
#define QWORD_WIDTH 64
#define DWORD_WIDTH 32
#define HALFWORD_WIDTH 8
#define POOL_STATUS_IS_OCCUPIED_BIT 0b00000001U
#define POOL_ELEMENT_COUNT 1024
#define POOL_STATUS_FLAGS_SIZE (POOL_ELEMENT_COUNT * HALFWORD_WIDTH)
#define POOL_MAP_ALLOC_UNIT 64
#define HEAP_SIZE 1024 * 64
#define BIG_HEAP_SIZE 1024 * 1024 * 4
#define BIG_BIG_HEAP_SIZE 1024 * 1024 * 256
#define FREE_LIST_ALLOC_COUNT 32
#define HTML_TAG_RESRC_STRIDE 16
#define HTML_TAG_COUNT 63
#define HTML_TAG_HMAP_SLOT_COUNT 2048
#define CSS_ATTRIBUTE_RESRC_STRIDE 32
#define CSS_ATTRIBUTE_COUNT 210
#define CSS_ATTRIBUTE_HMAP_SLOT_COUNT 2048
#define SEED_SIGNATURE_STRIDE 8

#ifndef MAX_CHILD_VAR_WIDTH
#define MAX_CHILD_VAR_WIDTH 8
#endif

typedef uint64_t qword;
typedef uint32_t dword;
typedef uint16_t word;
typedef uint8_t hword;

// typedef enum {
//     UPDATE_DOBJ_FLAG_TAG_NAME         = 0b0000000000000001,
//     UPDATE_DOBJ_FLAG_TAG_ID           = 0b0000000000000010,
//     UPDATE_DOBJ_FLAG_P_ARENA_ATTR     = 0b0000000000000100,
//     UPDATE_DOBJ_FLAG_ARENA_ATTR_SIZE  = 0b0000000000001000,
//     UPDATE_DOBJ_FLAG_P_ARENA_INNER    = 0b0000000000010000,
//     UPDATE_DOBJ_FLAG_ARENA_INNER_SIZE = 0b0000000000100000,
//     UPDATE_DOBJ_FLAG_P_PARENT         = 0b0000000001000000,
//     UPDATE_DOBJ_FLAG_LIFE_SPAN_POINT  = 0b0000000010000000
// } UpdateDOBJFlags;
// typedef enum {
//     UPDATE_DOBJ_FLAG_GROUP_BUILD_DOM =
//     (
//         UPDATE_DOBJ_FLAG_LIFE_SPAN_POINT |
//         UPDATE_DOBJ_FLAG_P_ARENA_ATTR |
//         UPDATE_DOBJ_FLAG_ARENA_ATTR_SIZE |
//         UPDATE_DOBJ_FLAG_P_ARENA_INNER |
//         UPDATE_DOBJ_FLAG_ARENA_INNER_SIZE
//     ),
// } UpdateDOBJFlagGroups;

typedef uint16_t htmltag_t;
typedef struct {
    uint16_t HTML_HTML;
    uint16_t HTML_HEAD;
    uint16_t HTML_BODY;
    uint16_t HTML_TITLE;
    uint16_t HTML_LINK;
    uint16_t HTML_STYLE;
    uint16_t HTML_SCRIPT;
    uint16_t HTML_META;
    uint16_t HTML_DIV;
    uint16_t HTML_P;
    uint16_t HTML_BR;
    uint16_t HTML_HR;
    uint16_t HTML_H1;
    uint16_t HTML_H2;
    uint16_t HTML_H3;
    uint16_t HTML_H4;
    uint16_t HTML_H5;
    uint16_t HTML_H6;
    uint16_t HTML_A;
    uint16_t HTML_IMG;
    uint16_t HTML_UL;
    uint16_t HTML_OL;
    uint16_t HTML_LI;
    uint16_t HTML_TABLE;
    uint16_t HTML_THEAD;
    uint16_t HTML_TBODY;
    uint16_t HTML_TFOOT;
    uint16_t HTML_TR;
    uint16_t HTML_TH;
    uint16_t HTML_TD;
    uint16_t HTML_FORM;
    uint16_t HTML_INPUT;
    uint16_t HTML_TEXTAREA;
    uint16_t HTML_BUTTON;
    uint16_t HTML_SELECT;
    uint16_t HTML_OPTION;
    uint16_t HTML_LABEL;
    uint16_t HTML_HEADER;
    uint16_t HTML_NAV;
    uint16_t HTML_MAIN;
    uint16_t HTML_SECTION;
    uint16_t HTML_ARTICLE;
    uint16_t HTML_ASIDE;
    uint16_t HTML_FOOTER;
    uint16_t HTML_VIDEO;
    uint16_t HTML_AUDIO;
    uint16_t HTML_SOURCE;
    uint16_t HTML_CANVAS;
    uint16_t HTML_SVG;
    uint16_t HTML_DETAILS;
    uint16_t HTML_SUMMARY;
    uint16_t HTML_IFRAME;
    uint16_t HTML_PRE;
    uint16_t HTML_CODE;
    uint16_t HTML_BLOCKQUOTE;
    uint16_t HTML_STRONG;
    uint16_t HTML_EM;
    uint16_t HTML_B;
    uint16_t HTML_I;
    uint16_t HTML_U;
    uint16_t HTML_SMALL;
    uint16_t HTML_SUB;
    uint16_t HTML_SUP;
} HTMLtag;

typedef struct {
    uint16_t CSS_MARGIN_TOP;
    uint16_t CSS_MARGIN_RIGHT;
    uint16_t CSS_MARGIN_BOTTOM;
    uint16_t CSS_MARGIN_LEFT;

    uint16_t CSS_PADDING_TOP;
    uint16_t CSS_PADDING_RIGHT;
    uint16_t CSS_PADDING_BOTTOM;
    uint16_t CSS_PADDING_LEFT;

    uint16_t CSS_BORDER_TOP_WIDTH;
    uint16_t CSS_BORDER_RIGHT_WIDTH;
    uint16_t CSS_BORDER_BOTTOM_WIDTH;
    uint16_t CSS_BORDER_LEFT_WIDTH;

    uint16_t CSS_BORDER_TOP_STYLE;
    uint16_t CSS_BORDER_RIGHT_STYLE;
    uint16_t CSS_BORDER_BOTTOM_STYLE;
    uint16_t CSS_BORDER_LEFT_STYLE;

    uint16_t CSS_BORDER_TOP_COLOR;
    uint16_t CSS_BORDER_RIGHT_COLOR;
    uint16_t CSS_BORDER_BOTTOM_COLOR;
    uint16_t CSS_BORDER_LEFT_COLOR;

    uint16_t CSS_BORDER_TOP_LEFT_RADIUS;
    uint16_t CSS_BORDER_TOP_RIGHT_RADIUS;
    uint16_t CSS_BORDER_BOTTOM_RIGHT_RADIUS;
    uint16_t CSS_BORDER_BOTTOM_LEFT_RADIUS;

    uint16_t CSS_BORDER_COLLAPSE;
    uint16_t CSS_BORDER_SPACING;

    uint16_t CSS_BORDER_IMAGE_SOURCE;
    uint16_t CSS_BORDER_IMAGE_SLICE;
    uint16_t CSS_BORDER_IMAGE_WIDTH;
    uint16_t CSS_BORDER_IMAGE_OUTSET;
    uint16_t CSS_BORDER_IMAGE_REPEAT;

    uint16_t CSS_OUTLINE_COLOR;
    uint16_t CSS_OUTLINE_STYLE;
    uint16_t CSS_OUTLINE_WIDTH;
    uint16_t CSS_OUTLINE_OFFSET;

    uint16_t CSS_WIDTH;
    uint16_t CSS_MIN_WIDTH;
    uint16_t CSS_MAX_WIDTH;
    uint16_t CSS_HEIGHT;
    uint16_t CSS_MIN_HEIGHT;
    uint16_t CSS_MAX_HEIGHT;
    uint16_t CSS_BOX_SIZING;
    uint16_t CSS_ASPECT_RATIO;

    uint16_t CSS_POSITION;
    uint16_t CSS_TOP;
    uint16_t CSS_RIGHT;
    uint16_t CSS_BOTTOM;
    uint16_t CSS_LEFT;
    uint16_t CSS_Z_INDEX;
    uint16_t CSS_FLOAT;
    uint16_t CSS_CLEAR;

    uint16_t CSS_DISPLAY;
    uint16_t CSS_VISIBILITY;
    uint16_t CSS_OPACITY;
    uint16_t CSS_OVERFLOW_X;
    uint16_t CSS_OVERFLOW_Y;
    uint16_t CSS_CLIP;
    uint16_t CSS_CLIP_PATH;
    uint16_t CSS_OBJECT_FIT;
    uint16_t CSS_OBJECT_POSITION;

    uint16_t CSS_BACKGROUND_COLOR;
    uint16_t CSS_BACKGROUND_IMAGE;
    uint16_t CSS_BACKGROUND_REPEAT;
    uint16_t CSS_BACKGROUND_POSITION_X;
    uint16_t CSS_BACKGROUND_POSITION_Y;
    uint16_t CSS_BACKGROUND_SIZE;
    uint16_t CSS_BACKGROUND_ATTACHMENT;
    uint16_t CSS_BACKGROUND_ORIGIN;
    uint16_t CSS_BACKGROUND_CLIP;
    uint16_t CSS_BACKGROUND_BLEND_MODE;

    uint16_t CSS_COLOR;

    uint16_t CSS_FONT_FAMILY;
    uint16_t CSS_FONT_SIZE;
    uint16_t CSS_FONT_STYLE;
    uint16_t CSS_FONT_WEIGHT;
    uint16_t CSS_FONT_VARIANT;
    uint16_t CSS_FONT_STRETCH;
    uint16_t CSS_FONT_SIZE_ADJUST;
    uint16_t CSS_FONT_KERNING;
    uint16_t CSS_FONT_FEATURE_SETTINGS;
    uint16_t CSS_FONT_VARIATION_SETTINGS;
    uint16_t CSS_LINE_HEIGHT;

    uint16_t CSS_TEXT_ALIGN;
    uint16_t CSS_TEXT_ALIGN_LAST;
    uint16_t CSS_TEXT_INDENT;
    uint16_t CSS_TEXT_TRANSFORM;
    uint16_t CSS_TEXT_OVERFLOW;
    uint16_t CSS_TEXT_SHADOW;
    uint16_t CSS_TEXT_JUSTIFY;
    uint16_t CSS_TEXT_DECORATION_LINE;
    uint16_t CSS_TEXT_DECORATION_STYLE;
    uint16_t CSS_TEXT_DECORATION_COLOR;
    uint16_t CSS_TEXT_DECORATION_THICKNESS;
    uint16_t CSS_TEXT_UNDERLINE_OFFSET;
    uint16_t CSS_LETTER_SPACING;
    uint16_t CSS_WORD_SPACING;
    uint16_t CSS_WHITE_SPACE;
    uint16_t CSS_WORD_BREAK;
    uint16_t CSS_WORD_WRAP;
    uint16_t CSS_OVERFLOW_WRAP;
    uint16_t CSS_HYPHENS;
    uint16_t CSS_TAB_SIZE;
    uint16_t CSS_DIRECTION;
    uint16_t CSS_UNICODE_BIDI;
    uint16_t CSS_WRITING_MODE;
    uint16_t CSS_VERTICAL_ALIGN;

    uint16_t CSS_FLEX_DIRECTION;
    uint16_t CSS_FLEX_WRAP;
    uint16_t CSS_FLEX_GROW;
    uint16_t CSS_FLEX_SHRINK;
    uint16_t CSS_FLEX_BASIS;
    uint16_t CSS_ORDER;
    uint16_t CSS_JUSTIFY_CONTENT;
    uint16_t CSS_JUSTIFY_ITEMS;
    uint16_t CSS_JUSTIFY_SELF;
    uint16_t CSS_ALIGN_CONTENT;
    uint16_t CSS_ALIGN_ITEMS;
    uint16_t CSS_ALIGN_SELF;
    uint16_t CSS_ROW_GAP;
    uint16_t CSS_COLUMN_GAP;

    uint16_t CSS_GRID_TEMPLATE_COLUMNS;
    uint16_t CSS_GRID_TEMPLATE_ROWS;
    uint16_t CSS_GRID_TEMPLATE_AREAS;
    uint16_t CSS_GRID_AUTO_COLUMNS;
    uint16_t CSS_GRID_AUTO_ROWS;
    uint16_t CSS_GRID_AUTO_FLOW;
    uint16_t CSS_GRID_ROW_START;
    uint16_t CSS_GRID_ROW_END;
    uint16_t CSS_GRID_COLUMN_START;
    uint16_t CSS_GRID_COLUMN_END;

    uint16_t CSS_COLUMN_COUNT;
    uint16_t CSS_COLUMN_WIDTH;
    uint16_t CSS_COLUMN_FILL;
    uint16_t CSS_COLUMN_SPAN;
    uint16_t CSS_COLUMN_RULE_WIDTH;
    uint16_t CSS_COLUMN_RULE_STYLE;
    uint16_t CSS_COLUMN_RULE_COLOR;

    uint16_t CSS_LIST_STYLE_TYPE;
    uint16_t CSS_LIST_STYLE_POSITION;
    uint16_t CSS_LIST_STYLE_IMAGE;

    uint16_t CSS_TABLE_LAYOUT;
    uint16_t CSS_CAPTION_SIDE;
    uint16_t CSS_EMPTY_CELLS;

    uint16_t CSS_CONTENT;
    uint16_t CSS_QUOTES;
    uint16_t CSS_COUNTER_RESET;
    uint16_t CSS_COUNTER_INCREMENT;

    uint16_t CSS_TRANSFORM;
    uint16_t CSS_TRANSFORM_ORIGIN;
    uint16_t CSS_TRANSFORM_STYLE;
    uint16_t CSS_PERSPECTIVE;
    uint16_t CSS_PERSPECTIVE_ORIGIN;
    uint16_t CSS_BACKFACE_VISIBILITY;
    uint16_t CSS_TRANSLATE;
    uint16_t CSS_ROTATE;
    uint16_t CSS_SCALE;

    uint16_t CSS_TRANSITION_PROPERTY;
    uint16_t CSS_TRANSITION_DURATION;
    uint16_t CSS_TRANSITION_TIMING_FUNCTION;
    uint16_t CSS_TRANSITION_DELAY;

    uint16_t CSS_ANIMATION_NAME;
    uint16_t CSS_ANIMATION_DURATION;
    uint16_t CSS_ANIMATION_TIMING_FUNCTION;
    uint16_t CSS_ANIMATION_DELAY;
    uint16_t CSS_ANIMATION_ITERATION_COUNT;
    uint16_t CSS_ANIMATION_DIRECTION;
    uint16_t CSS_ANIMATION_FILL_MODE;
    uint16_t CSS_ANIMATION_PLAY_STATE;

    uint16_t CSS_BOX_SHADOW;
    uint16_t CSS_FILTER;
    uint16_t CSS_BACKDROP_FILTER;
    uint16_t CSS_MIX_BLEND_MODE;
    uint16_t CSS_ISOLATION;

    uint16_t CSS_MASK_IMAGE;
    uint16_t CSS_MASK_MODE;
    uint16_t CSS_MASK_REPEAT;
    uint16_t CSS_MASK_POSITION;
    uint16_t CSS_MASK_SIZE;
    uint16_t CSS_MASK_ORIGIN;
    uint16_t CSS_MASK_CLIP;
    uint16_t CSS_MASK_COMPOSITE;

    uint16_t CSS_SCROLL_BEHAVIOR;
    uint16_t CSS_SCROLL_SNAP_TYPE;
    uint16_t CSS_SCROLL_SNAP_ALIGN;
    uint16_t CSS_SCROLL_MARGIN_TOP;
    uint16_t CSS_SCROLL_MARGIN_RIGHT;
    uint16_t CSS_SCROLL_MARGIN_BOTTOM;
    uint16_t CSS_SCROLL_MARGIN_LEFT;
    uint16_t CSS_SCROLL_PADDING_TOP;
    uint16_t CSS_SCROLL_PADDING_RIGHT;
    uint16_t CSS_SCROLL_PADDING_BOTTOM;
    uint16_t CSS_SCROLL_PADDING_LEFT;
    uint16_t CSS_OVERSCROLL_BEHAVIOR_X;
    uint16_t CSS_OVERSCROLL_BEHAVIOR_Y;

    uint16_t CSS_CURSOR;
    uint16_t CSS_POINTER_EVENTS;
    uint16_t CSS_USER_SELECT;
    uint16_t CSS_RESIZE;
    uint16_t CSS_TOUCH_ACTION;
    uint16_t CSS_CARET_COLOR;
    uint16_t CSS_ACCENT_COLOR;
    uint16_t CSS_APPEARANCE;

    uint16_t CSS_WILL_CHANGE;
    uint16_t CSS_CONTAIN;
    uint16_t CSS_IMAGE_RENDERING;
    uint16_t CSS_PAGE_BREAK_BEFORE;
    uint16_t CSS_PAGE_BREAK_AFTER;
    uint16_t CSS_PAGE_BREAK_INSIDE;
    uint16_t CSS_ORPHANS;
    uint16_t CSS_WIDOWS;
} CSSattribute;

typedef enum {
    CSS_ATTRIBUTE_DISPLAY_BLOCK,
    CSS_ATTRIBUTE_DISPLAY_NONE,
    CSS_ATTRIBUTE_DISPLAY_INLINE,
    CSS_ATTRIBUTE_DISPLAY_INLINE_BLOCK,
    CSS_ATTRIBUTE_DISPLAY_FLEX,
    CSS_ATTRIBUTE_DISPLAY_GRID
} CSSattributeDisplay;

typedef uint8_t CSSattributePosition; enum {
    CSS_ATTRIBUTE_POSITION_STATIC,
    CSS_ATTRIBUTE_POSITION_RELATIVE,
    CSS_ATTRIBUTE_POSITION_ABSOLUTE,
    CSS_ATTRIBUTE_POSITION_FIXED,
    CSS_ATTRIBUTE_POSITION_STICKY
};

typedef enum {
    CSS_ATTRIBUTE_BOX_SIZING_BORDER_BOX,
    CSS_ATTRIBUTE_BOX_SIZING_CONTENT_BOX
} CSSattributeBoxSizing;

typedef enum {
    CSS_ATTRIBUTE_DIRECTION_LTR,
    CSS_ATTRIBUTE_DIRECTION_RTL
} CSSattributeDirection;

typedef enum {
    CSS_ATTRIBUTE_OVERFLOW_VISIBLE,
    CSS_ATTRIBUTE_OVERFLOW_CLIP,
    CSS_ATTRIBUTE_OVERFLOW_HIDDEN,
    CSS_ATTRIBUTE_OVERFLOW_SCROLL
} CSSattributeOverflow;

typedef enum {
    CSS_ATTRIBUTE_TEXT_ALIGN_AUTO,
    CSS_ATTRIBUTE_TEXT_ALIGN_LEGACY_LEFT,
    CSS_ATTRIBUTE_TEXT_ALIGN_LEGACY_RIGHT,
    CSS_ATTRIBUTE_TEXT_ALIGN_LEGACY_CENTER
} CSSattributeTextAlign;
//flexbox
typedef enum {
    CSS_ATTRIBUTE_ALIGN_ITEMS_KEYWORD_NORMAL,
    CSS_ATTRIBUTE_ALIGN_ITEMS_KEYWORD_START,
    CSS_ATTRIBUTE_ALIGN_ITEMS_KEYWORD_END,
    CSS_ATTRIBUTE_ALIGN_ITEMS_KEYWORD_FLEX_START,
    CSS_ATTRIBUTE_ALIGN_ITEMS_KEYWORD_FLEX_END,
    CSS_ATTRIBUTE_ALIGN_ITEMS_KEYWORD_UNSET
} CSSattributeAlignItemsKeyword;

typedef enum {
    CSS_ATTRIBUTE_ALIGNMENT_SAFETY_DEFAULT,
    CSS_ATTRIBUTE_ALIGNMENT_SAFETY_SAFE,
    CSS_ATTRIBUTE_ALIGNMENT_SAFETY_UNSAFE
} CSSattributeAlignmentSafety;

typedef struct {
    CSSattributeAlignItemsKeyword keyword;
    CSSattributeAlignmentSafety safety;
} AlignItems;

typedef AlignItems AlignSelf;

typedef enum {
    CSS_ATTRIBUTE_FLEX_DIRECTION_ROW,
    CSS_ATTRIBUTE_FLEX_DIRECTION_COLUMN,
    CSS_ATTRIBUTE_FLEX_DIRECTION_ROW_REVERSE,
    CSS_ATTRIBUTE_FLEX_DIRECTION_COLUMN_REVERSE
} CSSattributeFlexDirection;

typedef enum {
    CSS_ATTRIBUTE_FLEX_WRAP_NO_WRAP,
    CSS_ATTRIBUTE_FLEX_WRAP_WRAP,
    CSS_ATTRIBUTE_FLEX_WRAP_WRAP_REVERSE,
    CSS_ATTRIBUTE_FLEX_WRAP_BALANCE,
    CSS_ATTRIBUTE_FLEX_WRAP_BALANCE_REVERSE
} CSSattributeFlexWrap;

typedef uint16_t HeapAddress16;
typedef uint64_t HeapAddress64;
//memory managment
typedef struct {
    void *heap;
    HeapAddress16 a; //in page size
    uint16_t s; //in byte
} HeapHandler16;
typedef struct {
    HeapAddress16 a; //in page size
    uint16_t s; //in word
} HeapChunkHandler16;
typedef struct {
    void *heap;
    HeapAddress64 a; //in page size
    uint64_t s; //in byte
} HeapHandler64;
typedef struct {
    HeapAddress64 a; //in page size
    uint64_t s; //in qword
} HeapChunkHandler64;

typedef struct {
    void *p_prev;
    void *p_next;
    SRWLOCK free_list_lock;
    uint64_t free_list_version;
    HeapChunkHandler16 *free_list;

    uint16_t heap_index;
    uint16_t live_objects;
    uint16_t free_list_alloc_count;
    uint16_t free_list_count;
    uint16_t heap_free_list_cache_h[256]; //hashed head local address of the more significant free chunk
    uint16_t heap_free_list_cache_t[256]; //hashed tail local address of the less significant free chunk
    uint16_t mem[HEAP_SIZE / 2]; //size: 64KB, min addressable size: 2B, minimum allocatable chunk is sizeof(mem) / (UINT16_MAX + 1) = 2B
} Heap;
typedef struct {
    void *p_prev;
    void *p_next;
    SRWLOCK free_list_lock;
    uint64_t free_list_version;
    HeapChunkHandler64 *free_list;

    uint16_t heap_index;
    uint8_t live_objects;
    uint8_t free_list_alloc_count;
    uint8_t free_list_count;
    uint8_t heap_free_list_cache_h[256];
    uint8_t heap_free_list_cache_t[256];

    uint64_t mem[BIG_HEAP_SIZE / 8]; //size: 4MB, min addressable size: 8B, desired minimum allocatable chunk is sizeof(mem) / (UINT8_MAX + 1) = 16KB
} BHeap;
typedef struct {
    void *p_prev;
    void *p_next;
    SRWLOCK free_list_lock;
    uint64_t free_list_version;
    HeapChunkHandler64 *free_list;

    uint16_t heap_index;
    uint8_t live_objects;
    uint8_t free_list_alloc_count;
    uint8_t free_list_count;
    uint8_t heap_free_list_cache_h[256];
    uint8_t heap_free_list_cache_t[256];

    uint64_t mem[BIG_BIG_HEAP_SIZE / 8]; //size: 256MB, page size: 8B, desired minimum allocatable chunk is sizeof(mem) / (UINT8_MAX + 1) = 1MB
} BBHeap;
//allocating less than desired minimum size may lead to higher chance of key collision of cache, resulting in reduced performance



typedef enum {
    HEAP_CLASS_HEAP = 1,
    HEAP_CLASS_BIG_HEAP = 2,
    HEAP_CLASS_BIG_BIG_HEAP = 3
} HeapClass;

typedef enum {
    DOBJ_PROPERTY_UNIT_OFFSET_WIDTH = 0,
    DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT = 3,
    DOBJ_PROPERTY_UNIT_OFFSET_TOP = 6,
    DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM = 9,
    DOBJ_PROPERTY_UNIT_OFFSET_LEFT = 12,
    DOBJ_PROPERTY_UNIT_OFFSET_RIGHT = 15,
    DOBJ_PROPERTY_UNIT_OFFSET_FONT = 18,
    DOBJ_PROPERTY_UNIT_OFFSET_MARGIN = 21,
    DOBJ_PROPERTY_UNIT_OFFSET_PADDING = 24,
    DOBJ_PROPERTY_UNIT_OFFSET_BORDER = 27
} DOBJpropertyUnitOffsets;

typedef enum {
    DOBJ_PROPERTY_UNIT_MASK_WIDTH = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_MASK_HEIGHT = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_MASK_TOP = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_MASK_BOTTOM = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_MASK_LEFT = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_MASK_RIGHT = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_MASK_FONT = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_MASK_MARGIN = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_MASK_PADDING = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_MASK_BORDER = 0b111UL << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
} DOBJpropertyUnitMask;

typedef uint32_t DOBJpropertyUnitFlag; enum {
    DOBJ_PROPERTY_UNIT_AUTO_GROUP_WIDTH_BIT       = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_WIDTH_BIT      = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_WIDTH_BIT     = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_WIDTH_BIT    = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_WIDTH_BIT   = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_WIDTH_BIT     = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_WIDTH_BIT       = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_WIDTH_BIT       = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_WIDTH,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_HEIGHT_BIT      = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_HEIGHT_BIT     = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_HEIGHT_BIT    = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_HEIGHT_BIT   = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_HEIGHT_BIT  = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_HEIGHT_BIT    = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_HEIGHT_BIT      = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_HEIGHT_BIT      = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_HEIGHT,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_TOP_BIT         = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_TOP_BIT        = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_TOP_BIT       = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_TOP_BIT      = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_TOP_BIT     = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_TOP_BIT       = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_TOP_BIT         = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_TOP_BIT         = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_TOP,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_BOTTOM_BIT      = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_BOTTOM_BIT     = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_BOTTOM_BIT    = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_BOTTOM_BIT   = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_BOTTOM_BIT  = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_BOTTOM_BIT    = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_BOTTOM_BIT      = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_BOTTOM_BIT      = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_BOTTOM,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_LEFT_BIT        = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_LEFT_BIT       = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_LEFT_BIT      = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_LEFT_BIT     = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_LEFT_BIT    = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_LEFT_BIT      = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_LEFT_BIT        = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_LEFT_BIT        = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_LEFT,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_RIGHT_BIT       = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_RIGHT_BIT      = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_RIGHT_BIT     = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_RIGHT_BIT    = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_RIGHT_BIT   = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_RIGHT_BIT     = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_RIGHT_BIT       = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_RIGHT_BIT       = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_RIGHT,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_FONT_BIT        = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_FONT_BIT       = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_FONT_BIT      = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_FONT_BIT     = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_FONT_BIT    = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_FONT_BIT      = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_FONT_BIT        = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_FONT_BIT        = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_FONT,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_MARGIN_BIT        = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_MARGIN_BIT       = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_MARGIN_BIT      = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_MARGIN_BIT     = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_MARGIN_BIT    = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_MARGIN_BIT      = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_MARGIN_BIT        = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_MARGIN_BIT        = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_MARGIN,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_PADDING_BIT        = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_PADDING_BIT       = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_PADDING_BIT      = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_PADDING_BIT     = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_PADDING_BIT    = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_PADDING_BIT      = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_PADDING_BIT        = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_PADDING_BIT        = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_PADDING,

    DOBJ_PROPERTY_UNIT_AUTO_GROUP_BORDER_BIT        = 0b000 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_PIXEL_GROUP_BORDER_BIT       = 0b001 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_VWIDTH_GROUP_BORDER_BIT      = 0b010 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_VHEIGHT_GROUP_BORDER_BIT     = 0b011 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_RELATIVE_GROUP_BORDER_BIT    = 0b100 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_F_CALC_GROUP_BORDER_BIT      = 0b101 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_VMIN_GROUP_BORDER_BIT        = 0b110 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER,
    DOBJ_PROPERTY_UNIT_VMAX_GROUP_BORDER_BIT        = 0b111 << DOBJ_PROPERTY_UNIT_OFFSET_BORDER
};

typedef struct {
    float top;
    float bottom;
    float left;
    float right;
} LayoutBoxProperties;

typedef uint16_t poid; //pool object id

typedef struct {
    struct DOBJPool *pool;
    poid item_index;
} DOBJHandler;

typedef struct {
    char tag_name[16];
    htmltag_t tag_id;
    HeapHandler16 p_attr;
    HeapHandler64 p_inner;
    DOBJHandler p_parent;
    uint16_t child_count;

    CSSattributePosition position;
    float x;
    float y;
    float width;
    float height;

    LayoutBoxProperties inset;
    LayoutBoxProperties padding;
    LayoutBoxProperties margin;
} DOBJ;


//conceptually
/*
    uint16_t pool_index;
    uint16_t live_objects;
    struct DOBJPool *p_prev;
    struct DOBJPool *p_next;
    uint8_t status_flags[POOL_ELEMENT_COUNT];

    char tag_names[POOL_ELEMENT_COUNT][16];
    htmltag_t tag_ids[POOL_ELEMENT_COUNT];
    HeapHandler16 p_attrs[POOL_ELEMENT_COUNT];
    size_t attr_sizes[POOL_ELEMENT_COUNT];
    HeapHandler64 p_inner[POOL_ELEMENT_COUNT];
    DOBJHandler parent[POOL_ELEMENT_COUNT];
    uint16_t child_count[POOL_ELEMENT_COUNT];
    uint8_t life_span_points[POOL_ELEMENT_COUNT];

    DOBJpropertiesIsSetGroup1Flag is_set_g1[POOL_ELEMENT_COUNT];

    CSSattributePosition position[POOL_ELEMENT_COUNT];
    float x[POOL_ELEMENT_COUNT];
    float y[POOL_ELEMENT_COUNT];
    float width[POOL_ELEMENT_COUNT];
    float height[POOL_ELEMENT_COUNT];

    LayoutBoxProperties inset[POOL_ELEMENT_COUNT];
    LayoutBoxProperties padding[POOL_ELEMENT_COUNT];
    LayoutBoxProperties margin[POOL_ELEMENT_COUNT];
*/

typedef struct DOBJPool {
    uint16_t pool_index;
    uint16_t live_objects;
    struct DOBJPool *p_prev;
    struct DOBJPool *p_next;
    uint8_t status_flags[POOL_ELEMENT_COUNT];

    char tag_names[POOL_ELEMENT_COUNT][16];
    htmltag_t tag_ids[POOL_ELEMENT_COUNT];
    //HeapHandler16 p_attrs[POOL_ELEMENT_COUNT];
    Heap *HeapHandler16_heap_h_attrs[POOL_ELEMENT_COUNT];
    HeapAddress16 HeapHandler16_a_h_attrs[POOL_ELEMENT_COUNT];
    uint16_t HeapHandler16_s_h_attrs[POOL_ELEMENT_COUNT];

    //HeapHandler64 p_inner[POOL_ELEMENT_COUNT];
    BHeap *HeapHandler64_heap_h_inner[POOL_ELEMENT_COUNT];
    HeapAddress64 HeapHandler64_a_h_inner[POOL_ELEMENT_COUNT];
    uint64_t HeapHandler64_s_h_inner[POOL_ELEMENT_COUNT];

    //DOBJHandler parent[POOL_ELEMENT_COUNT];
    struct DOBJPool *DOBJHandler_pool_parent[POOL_ELEMENT_COUNT];
    poid DOBJHandler_item_index_parent[POOL_ELEMENT_COUNT];

    UINTx_T(MAX_CHILD_VAR_WIDTH) child_count[POOL_ELEMENT_COUNT];
    //uint8_t life_span_points[POOL_ELEMENT_COUNT]; original design involved a GC, but no, so now its not needed anymore, everything is explicit

    CSSattributePosition position[POOL_ELEMENT_COUNT];
    float x[POOL_ELEMENT_COUNT];
    float y[POOL_ELEMENT_COUNT];
    float width[POOL_ELEMENT_COUNT];
    float height[POOL_ELEMENT_COUNT];

    LayoutBoxProperties inset[POOL_ELEMENT_COUNT];
    LayoutBoxProperties padding[POOL_ELEMENT_COUNT];
    LayoutBoxProperties margin[POOL_ELEMENT_COUNT];

} DOBJPool;

static inline HeapHandler16 dobj_get_h_attrs(DOBJHandler h)
{
    DOBJPool *p = h.pool;
    poid i = h.item_index;
    return (HeapHandler16){.heap = p->HeapHandler16_heap_h_attrs[i], .a = p->HeapHandler16_a_h_attrs[i], .s = p->HeapHandler16_s_h_attrs[i]};
}
static inline HeapHandler64 dobj_get_h_inner(DOBJHandler h)
{
    DOBJPool *p = h.pool;
    poid i = h.item_index;
    return (HeapHandler64){.heap = p->HeapHandler64_heap_h_inner[i], .a = p->HeapHandler64_a_h_inner[i], .s = p->HeapHandler64_s_h_inner[i]};
}
static inline DOBJHandler dobj_get_h_parent(DOBJHandler h)
{
    DOBJPool *p = h.pool;
    poid i = h.item_index;
    return (DOBJHandler){.pool = p->DOBJHandler_pool_parent[i], .item_index = p->DOBJHandler_item_index_parent[i]};
}

static inline LayoutBoxProperties padding(float t, float r, float b, float l)
{
    return (LayoutBoxProperties){.top = t, .bottom = b, .left = l, .right = r};
}
static inline LayoutBoxProperties margin(float t, float r, float b, float l)
{
    return (LayoutBoxProperties){.top = t, .bottom = b, .left = l, .right = r};
}
static inline uint16_t html_tag_hmap_id_get(const char* str)
{
    return hash_2_index(hash64(fnv1a_64(str, strlen(str)), SEED_HTML_TAG), HTML_TAG_HMAP_SLOT_COUNT);
}
static inline uint16_t html_tagn_hmap_id_get(const char* str, size_t len)
{
    return hash_2_index(hash64(fnv1a_64(str, len), SEED_HTML_TAG), HTML_TAG_HMAP_SLOT_COUNT);
}
static inline uint16_t css_attr_hmap_id_get(const char* str)
{
    return hash_2_index(hash64(fnv1a_64(str, strlen(str)), SEED_CSS_ATTRIBUTE), CSS_ATTRIBUTE_HMAP_SLOT_COUNT);
}
static inline uint16_t css_attrn_hmap_id_get(const char* str, size_t len)
{
    return hash_2_index(hash64(fnv1a_64(str, len), SEED_CSS_ATTRIBUTE), CSS_ATTRIBUTE_HMAP_SLOT_COUNT);
}

#endif