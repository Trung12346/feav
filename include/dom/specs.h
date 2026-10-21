#ifndef DOM_SPECS_H
#define DOM_SPECS_H

#include <stdint.h>
#include <stdatomic.h>
#include <windows.h>
#include "hardware_utils/hash.h"

#define QWORD_SCAN_IS_OCCUPIED_MASK 0b0000000100000001000000010000000100000001000000010000000100000001ULL
#define DWORD_SCAN_IS_OCCUPIED_MASK 0b00000001000000010000000100000001UL
#define HALFWORD_SCAN_IS_OCCUPIED_MASK 0b00000001U
#define QWORD_WIDTH 64
#define DWORD_WIDTH 32
#define HALFWORD_WIDTH 8
#define POOL_STATUS_IS_OCCUPIED_BIT 0b00000001U
#define POOL_ELEMENT_COUNT 1024
#define POOL_STATUS_FLAGS_SIZE (POOL_ELEMENT_COUNT * HALFWORD_WIDTH)
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
#define SEED_SIGNATURE_WIDTH 8

typedef uint64_t qword;
typedef uint32_t dword;
typedef uint16_t word;
typedef uint8_t hword;

typedef enum {
    UPDATE_DOBJ_FLAG_TAG_NAME         = 0b0000000000000001,
    UPDATE_DOBJ_FLAG_TAG_ID           = 0b0000000000000010,
    UPDATE_DOBJ_FLAG_P_ARENA_ATTR     = 0b0000000000000100,
    UPDATE_DOBJ_FLAG_ARENA_ATTR_SIZE  = 0b0000000000001000,
    UPDATE_DOBJ_FLAG_P_ARENA_INNER    = 0b0000000000010000,
    UPDATE_DOBJ_FLAG_ARENA_INNER_SIZE = 0b0000000000100000,
    UPDATE_DOBJ_FLAG_P_PARENT         = 0b0000000001000000,
    UPDATE_DOBJ_FLAG_LIFE_SPAN_POINT  = 0b0000000010000000
} UpdateDOBJFlags;
typedef enum {
    UPDATE_DOBJ_FLAG_GROUP_BUILD_DOM =
    (
        UPDATE_DOBJ_FLAG_LIFE_SPAN_POINT |
        UPDATE_DOBJ_FLAG_P_ARENA_ATTR |
        UPDATE_DOBJ_FLAG_ARENA_ATTR_SIZE |
        UPDATE_DOBJ_FLAG_P_ARENA_INNER |
        UPDATE_DOBJ_FLAG_ARENA_INNER_SIZE
    ),
} UpdateDOBJFlagGroups;

typedef enum {
    CUSTOM      = 0,
    HTML        = 1,
    HEAD        = 2,
    BODY        = 3,
    TITLE       = 4,
    LINK        = 5,
    STYLE       = 6,
    SCRIPT      = 7,
    META        = 8,
    DIV         = 9,
    P           = 10,
    BR          = 11,
    HR          = 12,
    H1          = 13,
    H2          = 14,
    H3          = 15,
    H4          = 16,
    H5          = 17,
    H6          = 18,
    A           = 19,
    IMG         = 20,
    UL          = 21,
    OL          = 22,
    LI          = 23,
    TABLE       = 24,
    THEAD       = 25,
    TBODY       = 26,
    TFOOT       = 27,
    TR          = 28,
    TH          = 29,
    TD          = 30,
    FORM        = 31,
    DINPUT      = 32,
    TEXTAREA    = 33,
    BUTTON      = 34,
    SELECT      = 35,
    OPTION      = 36,
    LABEL       = 37,
    HEADER      = 38,
    NAV         = 39,
    MAIN        = 40,
    SECTION     = 41,
    ARTICLE     = 42,
    ASIDE       = 43,
    FOOTER      = 44,
    VIDEO       = 45,
    AUDIO       = 46,
    SOURCE      = 47,
    CANVAS      = 48,
    SVG         = 49,
    DETAILS     = 50,
    SUMMARY     = 51,
    IFRAME      = 52,
    PRE         = 53,
    CODE        = 54,
    BLOCKQUOTE  = 55,
    STRONG      = 56,
    EM          = 57,
    B           = 58,
    I           = 59,
    U           = 60,
    SMALL       = 61,
    SUB         = 62,
    SUP         = 63
} TagIdentifier;

enum {
    /* Margin */
    CSS_MARGIN_TOP = 1,
    CSS_MARGIN_RIGHT,
    CSS_MARGIN_BOTTOM,
    CSS_MARGIN_LEFT,
 
    /* Padding */
    CSS_PADDING_TOP,
    CSS_PADDING_RIGHT,
    CSS_PADDING_BOTTOM,
    CSS_PADDING_LEFT,
 
    /* Border width */
    CSS_BORDER_TOP_WIDTH,
    CSS_BORDER_RIGHT_WIDTH,
    CSS_BORDER_BOTTOM_WIDTH,
    CSS_BORDER_LEFT_WIDTH,
 
    /* Border style */
    CSS_BORDER_TOP_STYLE,
    CSS_BORDER_RIGHT_STYLE,
    CSS_BORDER_BOTTOM_STYLE,
    CSS_BORDER_LEFT_STYLE,
 
    /* Border color */
    CSS_BORDER_TOP_COLOR,
    CSS_BORDER_RIGHT_COLOR,
    CSS_BORDER_BOTTOM_COLOR,
    CSS_BORDER_LEFT_COLOR,
 
    /* Border radius */
    CSS_BORDER_TOP_LEFT_RADIUS,
    CSS_BORDER_TOP_RIGHT_RADIUS,
    CSS_BORDER_BOTTOM_RIGHT_RADIUS,
    CSS_BORDER_BOTTOM_LEFT_RADIUS,
 
    /* Border collapse / table borders */
    CSS_BORDER_COLLAPSE,
    CSS_BORDER_SPACING,
 
    /* Border image */
    CSS_BORDER_IMAGE_SOURCE,
    CSS_BORDER_IMAGE_SLICE,
    CSS_BORDER_IMAGE_WIDTH,
    CSS_BORDER_IMAGE_OUTSET,
    CSS_BORDER_IMAGE_REPEAT,
 
    /* Outline */
    CSS_OUTLINE_COLOR,
    CSS_OUTLINE_STYLE,
    CSS_OUTLINE_WIDTH,
    CSS_OUTLINE_OFFSET,
 
    /* Box model / sizing */
    CSS_WIDTH,
    CSS_MIN_WIDTH,
    CSS_MAX_WIDTH,
    CSS_HEIGHT,
    CSS_MIN_HEIGHT,
    CSS_MAX_HEIGHT,
    CSS_BOX_SIZING,
    CSS_ASPECT_RATIO,
 
    /* Positioning */
    CSS_POSITION,
    CSS_TOP,
    CSS_RIGHT,
    CSS_BOTTOM,
    CSS_LEFT,
    CSS_Z_INDEX,
    CSS_FLOAT,
    CSS_CLEAR,
 
    /* Display / visibility */
    CSS_DISPLAY,
    CSS_VISIBILITY,
    CSS_OPACITY,
    CSS_OVERFLOW_X,
    CSS_OVERFLOW_Y,
    CSS_CLIP,
    CSS_CLIP_PATH,
    CSS_OBJECT_FIT,
    CSS_OBJECT_POSITION,
 
    /* Background */
    CSS_BACKGROUND_COLOR,
    CSS_BACKGROUND_IMAGE,
    CSS_BACKGROUND_REPEAT,
    CSS_BACKGROUND_POSITION_X,
    CSS_BACKGROUND_POSITION_Y,
    CSS_BACKGROUND_SIZE,
    CSS_BACKGROUND_ATTACHMENT,
    CSS_BACKGROUND_ORIGIN,
    CSS_BACKGROUND_CLIP,
    CSS_BACKGROUND_BLEND_MODE,
 
    /* Color */
    CSS_COLOR,
 
    /* Font */
    CSS_FONT_FAMILY,
    CSS_FONT_SIZE,
    CSS_FONT_STYLE,
    CSS_FONT_WEIGHT,
    CSS_FONT_VARIANT,
    CSS_FONT_STRETCH,
    CSS_FONT_SIZE_ADJUST,
    CSS_FONT_KERNING,
    CSS_FONT_FEATURE_SETTINGS,
    CSS_FONT_VARIATION_SETTINGS,
    CSS_LINE_HEIGHT,
 
    /* Text */
    CSS_TEXT_ALIGN,
    CSS_TEXT_ALIGN_LAST,
    CSS_TEXT_INDENT,
    CSS_TEXT_TRANSFORM,
    CSS_TEXT_OVERFLOW,
    CSS_TEXT_SHADOW,
    CSS_TEXT_JUSTIFY,
    CSS_TEXT_DECORATION_LINE,
    CSS_TEXT_DECORATION_STYLE,
    CSS_TEXT_DECORATION_COLOR,
    CSS_TEXT_DECORATION_THICKNESS,
    CSS_TEXT_UNDERLINE_OFFSET,
    CSS_LETTER_SPACING,
    CSS_WORD_SPACING,
    CSS_WHITE_SPACE,
    CSS_WORD_BREAK,
    CSS_WORD_WRAP,
    CSS_OVERFLOW_WRAP,
    CSS_HYPHENS,
    CSS_TAB_SIZE,
    CSS_DIRECTION,
    CSS_UNICODE_BIDI,
    CSS_WRITING_MODE,
    CSS_VERTICAL_ALIGN,
 
    /* Flexbox */
    CSS_FLEX_DIRECTION,
    CSS_FLEX_WRAP,
    CSS_FLEX_GROW,
    CSS_FLEX_SHRINK,
    CSS_FLEX_BASIS,
    CSS_ORDER,
    CSS_JUSTIFY_CONTENT,
    CSS_JUSTIFY_ITEMS,
    CSS_JUSTIFY_SELF,
    CSS_ALIGN_CONTENT,
    CSS_ALIGN_ITEMS,
    CSS_ALIGN_SELF,
    CSS_ROW_GAP,
    CSS_COLUMN_GAP,
 
    /* Grid */
    CSS_GRID_TEMPLATE_COLUMNS,
    CSS_GRID_TEMPLATE_ROWS,
    CSS_GRID_TEMPLATE_AREAS,
    CSS_GRID_AUTO_COLUMNS,
    CSS_GRID_AUTO_ROWS,
    CSS_GRID_AUTO_FLOW,
    CSS_GRID_ROW_START,
    CSS_GRID_ROW_END,
    CSS_GRID_COLUMN_START,
    CSS_GRID_COLUMN_END,
 
    /* Multi-column */
    CSS_COLUMN_COUNT,
    CSS_COLUMN_WIDTH,
    CSS_COLUMN_FILL,
    CSS_COLUMN_SPAN,
    CSS_COLUMN_RULE_WIDTH,
    CSS_COLUMN_RULE_STYLE,
    CSS_COLUMN_RULE_COLOR,
 
    /* Lists */
    CSS_LIST_STYLE_TYPE,
    CSS_LIST_STYLE_POSITION,
    CSS_LIST_STYLE_IMAGE,
 
    /* Tables */
    CSS_TABLE_LAYOUT,
    CSS_CAPTION_SIDE,
    CSS_EMPTY_CELLS,
 
    /* Generated content */
    CSS_CONTENT,
    CSS_QUOTES,
    CSS_COUNTER_RESET,
    CSS_COUNTER_INCREMENT,
 
    /* Transforms */
    CSS_TRANSFORM,
    CSS_TRANSFORM_ORIGIN,
    CSS_TRANSFORM_STYLE,
    CSS_PERSPECTIVE,
    CSS_PERSPECTIVE_ORIGIN,
    CSS_BACKFACE_VISIBILITY,
    CSS_TRANSLATE,
    CSS_ROTATE,
    CSS_SCALE,
 
    /* Transitions */
    CSS_TRANSITION_PROPERTY,
    CSS_TRANSITION_DURATION,
    CSS_TRANSITION_TIMING_FUNCTION,
    CSS_TRANSITION_DELAY,
 
    /* Animations */
    CSS_ANIMATION_NAME,
    CSS_ANIMATION_DURATION,
    CSS_ANIMATION_TIMING_FUNCTION,
    CSS_ANIMATION_DELAY,
    CSS_ANIMATION_ITERATION_COUNT,
    CSS_ANIMATION_DIRECTION,
    CSS_ANIMATION_FILL_MODE,
    CSS_ANIMATION_PLAY_STATE,
 
    /* Effects */
    CSS_BOX_SHADOW,
    CSS_FILTER,
    CSS_BACKDROP_FILTER,
    CSS_MIX_BLEND_MODE,
    CSS_ISOLATION,
 
    /* Mask */
    CSS_MASK_IMAGE,
    CSS_MASK_MODE,
    CSS_MASK_REPEAT,
    CSS_MASK_POSITION,
    CSS_MASK_SIZE,
    CSS_MASK_ORIGIN,
    CSS_MASK_CLIP,
    CSS_MASK_COMPOSITE,
 
    /* Scroll */
    CSS_SCROLL_BEHAVIOR,
    CSS_SCROLL_SNAP_TYPE,
    CSS_SCROLL_SNAP_ALIGN,
    CSS_SCROLL_MARGIN_TOP,
    CSS_SCROLL_MARGIN_RIGHT,
    CSS_SCROLL_MARGIN_BOTTOM,
    CSS_SCROLL_MARGIN_LEFT,
    CSS_SCROLL_PADDING_TOP,
    CSS_SCROLL_PADDING_RIGHT,
    CSS_SCROLL_PADDING_BOTTOM,
    CSS_SCROLL_PADDING_LEFT,
    CSS_OVERSCROLL_BEHAVIOR_X,
    CSS_OVERSCROLL_BEHAVIOR_Y,
 
    /* Interaction */
    CSS_CURSOR,
    CSS_POINTER_EVENTS,
    CSS_USER_SELECT,
    CSS_RESIZE,
    CSS_TOUCH_ACTION,
    CSS_CARET_COLOR,
    CSS_ACCENT_COLOR,
    CSS_APPEARANCE,
 
    /* Misc */
    CSS_WILL_CHANGE,
    CSS_CONTAIN,
    CSS_IMAGE_RENDERING,
    CSS_PAGE_BREAK_BEFORE,
    CSS_PAGE_BREAK_AFTER,
    CSS_PAGE_BREAK_INSIDE,
    CSS_ORPHANS,
    CSS_WIDOWS,
 
}; //claude helped with tis

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


//memory managment
typedef struct {
    void *heap;
    uint16_t a;
    uint16_t s; //in byte
} HeapHandler16;
typedef struct {
    uint16_t a;
    uint16_t s; //in word
} HeapChunkHandler16;
typedef struct {
    void *heap;
    uint64_t a;
    uint64_t s; //in byte
} HeapHandler64;
typedef struct {
    uint64_t a;
    uint64_t s; //in qword
} HeapChunkHandler64;

typedef struct {
    uint16_t heap_index;
    uint16_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler16 *free_list;
    uint16_t free_list_alloc_count;
    uint16_t free_list_count;
    SRWLOCK free_list_lock;
    uint64_t free_list_version;
    uint16_t heap_free_list_cache_h[256]; //hashed head local address of the more significant free chunk
    uint16_t heap_free_list_cache_t[256]; //hashed tail local address of the less significant free chunk

    uint16_t mem[HEAP_SIZE / 2]; //size: 64KB, page size: 2B, minimum allocatable chunk is sizeof(mem) / (UINT16_MAX + 1) = 2B
} Heap;
typedef struct {
    uint16_t heap_index;
    uint8_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler64 *free_list;
    uint8_t free_list_alloc_count;
    uint8_t free_list_count;
    SRWLOCK free_list_lock;
    uint64_t free_list_version;
    uint8_t heap_free_list_cache_h[256];
    uint8_t heap_free_list_cache_t[256];

    uint64_t mem[BIG_HEAP_SIZE / 8]; //size: 4MB, page size: 8B, desired minimum allocatable chunk is sizeof(mem) / (UINT8_MAX + 1) = 16KB
} BHeap;
typedef struct {
    uint16_t heap_index;
    uint8_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler64 *free_list;
    uint8_t free_list_alloc_count;
    uint8_t free_list_count;
    SRWLOCK free_list_lock;
    uint64_t free_list_version;
    uint8_t heap_free_list_cache_h[256];
    uint8_t heap_free_list_cache_t[256];
    uint8_t class;

    uint64_t mem[BIG_BIG_HEAP_SIZE / 8]; //size: 256MB, page size: 8B, desired minimum allocatable chunk is sizeof(mem) / (UINT8_MAX + 1) = 1MB
} BBHeap;
//allocating less than desired minimum size may lead to key collision of cache, resulting in reduced performance



typedef enum {
    HEAP_CLASS_HEAP = 1,
    HEAP_CLASS_BIG_HEAP = 2,
    HEAP_CLASS_BIG_BIG_HEAP = 3
} HeapClass;

//calling it group1 incase there need to be group2
typedef uint64_t DOBJpropertiesIsSetGroup1Flag; enum {
    DOBJ_PROPERTIES_IS_SET_GROUP1_WIDTH_BIT  = 0b1,
    DOBJ_PROPERTIES_IS_SET_GROUP1_HEIGHT_BIT = 0b10,
    DOBJ_PROPERTIES_IS_SET_GROUP1_TOP_BIT    = 0b100,
    DOBJ_PROPERTIES_IS_SET_GROUP1_BOTTOM_BIT = 0b1000,
    DOBJ_PROPERTIES_IS_SET_GROUP1_LEFT_BIT   = 0b10000,
    DOBJ_PROPERTIES_IS_SET_GROUP1_RIGHT_BIT  = 0b100000
};

typedef struct {
    float top;
    float bottom;
    float left;
    float right;
} LayoutBoxProperties;

typedef struct {
    struct DOBJPool *pool;
    uint16_t item_index;
} DOBJHandler;

typedef struct {
    char tag_name[256];
    TagIdentifier tag_id;
    HeapHandler16 p_attr;
    size_t attr_size;
    HeapHandler64 p_inner;
    DOBJHandler p_parent;
    uint16_t child_count;
    uint8_t life_span_point;

    DOBJpropertiesIsSetGroup1Flag is_set_g1;

    CSSattributePosition position;
    float x;
    float y;
    float width;
    float height;

    LayoutBoxProperties inset;
    LayoutBoxProperties padding;
    LayoutBoxProperties margin;
} DOBJ;

typedef struct DOBJPool {
    uint16_t pool_index;
    uint16_t live_objects;
    struct DOBJPool *p_prev;
    struct DOBJPool *p_next;
    uint8_t status_flags[POOL_ELEMENT_COUNT];

    char tag_names[POOL_ELEMENT_COUNT][256];
    TagIdentifier tag_ids[POOL_ELEMENT_COUNT];
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

} DOBJPool;



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

#endif