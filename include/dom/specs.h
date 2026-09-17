#ifndef DOM_SPECS_H
#define DOM_SPECS_H

#include <stdint.h>
#include <stdatomic.h>

#define QWORD_SCAN_IS_OCCUPIED_MASK 0b0000000100000001000000010000000100000001000000010000000100000001ULL
#define DWORD_SCAN_IS_OCCUPIED_MASK 0b00000001000000010000000100000001UL
#define HALFWORD_SCAN_IS_OCCUPIED_MASK 0b00000001U
#define QWORD_WIDTH 64
#define DWORD_WIDTH 32
#define HALFWORD_WIDTH 8
#define POOL_STATUS_IS_OCCUPIED_BIT 0b00000001U
#define POOL_ELEMENT_COUNT 1024
#define POOL_STATUS_FLAGS_SIZE POOL_ELEMENT_COUNT * HALFWORD_WIDTH
#define HEAP_SIZE 1024 * 128
#define BIG_HEAP_SIZE 1024 * 1024 * 8
#define BIG_BIG_HEAP_SIZE 1024 * 1024 * 512
#define FREE_LIST_ALLOC_COUNT 32

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
    CUSTOM      = 0,//
    HTML        = 1,//
    HEAD        = 2,//
    BODY        = 3,
    TITLE       = 4,//
    LINK        = 5,
    STYLE       = 6,
    SCRIPT      = 7,
    META        = 8,
    DIV         = 9,//
    P           = 10,//
    BR          = 11,
    HR          = 12,
    H1          = 13,//
    H2          = 14,//
    H3          = 15,//
    H4          = 16,//
    H5          = 17,//
    H6          = 18,//
    A           = 19,//
    IMG         = 20,//
    UL          = 21,//
    OL          = 22,//
    LI          = 23,//
    TABLE       = 24,//
    THEAD       = 25,//
    TBODY       = 26,//
    TFOOT       = 27,//
    TR          = 28,//
    TH          = 29,//
    TD          = 30,//
    FORM        = 31,
    DINPUT      = 32,//
    TEXTAREA    = 33,//
    BUTTON      = 34,
    SELECT      = 35,
    OPTION      = 36,
    LABEL       = 37,//
    HEADER      = 38,//
    NAV         = 39,//
    MAIN        = 40,
    SECTION     = 41,
    ARTICLE     = 42,
    ASIDE       = 43,
    FOOTER      = 44,
    VIDEO       = 45,
    AUDIO       = 46,
    SOURCE      = 47,
    CANVAS      = 48,
    SVG         = 49,//
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
    SUB         = 62,//
    SUP         = 63//
} TagIdentifier;



typedef struct {
    void *heap;
    uint16_t a;
    uint16_t s;
} HeapHandler16;
typedef struct {
    uint16_t a;
    uint16_t s;
} HeapChunkHandler16;
typedef struct {
    void *heap;
    uint64_t a;
    uint64_t s;
} HeapHandler64;
typedef struct {
    uint64_t a;
    uint64_t s;
} HeapChunkHandler64;

typedef struct {
    uint8_t class;
    uint16_t heap_index;
    uint16_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler16 *free_list;
    uint16_t free_list_alloc_count;
    uint16_t free_list_count;
    uint16_t heap_free_list_cache_h[HEAP_SIZE / 2];
    uint16_t heap_free_list_cache_t[HEAP_SIZE / 2];

    uint16_t mem[HEAP_SIZE / 2]; //size: 128KB, page size: 2B, (heap size / minimum allocatable size) = UINT16_MAX
} Heap;
typedef struct {
    uint8_t class;
    uint16_t heap_index;
    uint8_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler64 *free_list;
    uint8_t free_list_alloc_count;
    uint8_t free_list_count;
    uint8_t heap_free_list_cache_h[256];
    uint8_t heap_free_list_cache_t[256];

    uint64_t mem[BIG_HEAP_SIZE / 8]; //size: 8MB, page size: 8B, (heap size / minimum allocatable size) < UINT8_MAX
} BHeap;
typedef struct {
    uint8_t class;
    uint16_t heap_index;
    uint8_t live_objects;
    void *p_prev;
    void *p_next;
    HeapChunkHandler64 *free_list;
    uint8_t free_list_alloc_count;
    uint8_t free_list_count;
    uint8_t heap_free_list_cache_h[256];
    uint8_t heap_free_list_cache_t[256];
    uint8_t class;

    uint64_t mem[BIG_BIG_HEAP_SIZE / 8]; //size: 512MB, page size: 8B, (heap size / minimum allocatable size) < UINT8_MAX
} BBHeap;

typedef enum {
    HEAP_CLASS_HEAP = 1,
    HEAP_CLASS_BIG_HEAP = 2,
    HEAP_CLASS_BIG_BIG_HEAP = 3
} HeapClass;

typedef struct {
    char tag_name[256];
    TagIdentifier tag_id;
    void *p_attr;
    size_t attr_size;
    void *p_inner;
    size_t inner_size;
    void *p_parent;
    uint8_t life_span_point;
} DOBJ;

typedef struct {
    uint16_t pool_index;
    uint16_t live_objects;
    void *p_prev;
    void *p_next;
    uint8_t status_flags[POOL_ELEMENT_COUNT];

    char tag_names[POOL_ELEMENT_COUNT][256];
    TagIdentifier tag_ids[POOL_ELEMENT_COUNT];
    void *p_attrs[POOL_ELEMENT_COUNT];
    size_t attr_sizes[POOL_ELEMENT_COUNT];
    void *p_inner[POOL_ELEMENT_COUNT];
    size_t inner_size[POOL_ELEMENT_COUNT];
    void *p_parent[POOL_ELEMENT_COUNT];
    uint8_t life_span_points[POOL_ELEMENT_COUNT];
} DOBJPool;

typedef struct {
    DOBJPool *pool;
    uint16_t item_index;
} DOBJHandler;

#endif