#ifndef LAYOUT_ENGINE_SPECS_H
#define LAYOUT_ENGINE_SPECS_H

// layout_engine.h  --  C side of the Taffy layout integration (Rust crate in this folder).
//
// Needs in dom/specs.h (all there in the current version):
//   * DOBJpropertyUnitFlag is 64-bit, with per-side units and a DISPLAY slot;
//   * LayoutBoxProperties border[] at the end of the style arrays;
//   * ComputedElement { x, y, width, height, r, g, b, a } 32 bytes, aligned(8): the layout RESULT of one
//     element, stored in ONE flat array indexed by GLOBAL id (pool_index * POOL_ELEMENT_COUNT + item_index),
//     i.e. app->caches.results.
//
// Layout writes x, y, width, height of every element into that array and never touches r, g, b, a.
//
// Once at startup, so a struct mismatch fails loudly instead of producing garbage:
//
//     assert(feav_layout_pool_size() == sizeof(DOBJPool));
//     assert(feav_layout_computed_element_size() == sizeof(ComputedElement));
//
// The window size goes in the ROOT element's slot BEFORE every layout call (also after a resize):
//
//     app->caches.results[0].width  = windowWidth;
//     app->caches.results[0].height = windowHeight;
//
// IMPORTANT: every flag constant must be computed with 64-bit arithmetic. `0b001 << 33` is a
// 32-bit shift and gives 0; write `0b001ULL << 33`. (`UL` is not enough on Windows: there
// `unsigned long` is only 32 bits.)
//
// Display is NOT a separate array: it is the 3-bit code in prty_flags at
// DOBJ_PROPERTY_UNIT_OFFSET_DISPLAY (DOBJ_PROPERTY_UNIT_DISPLAY_*_BIT). 0 = unset, which means
// "use the browser default for this tag" (for example <head> and <script> are hidden).
// INLINE, INLINE_BLOCK and GRID are laid out as block until inline text flow / grid properties exist.

#include <stddef.h>
#include <stdint.h>

#include "dom/specs.h"

// How wide may a text be? Passed to the text measure callback as an int32_t.
typedef enum LayoutAvailableSpaceMode {
    LAYOUT_AVAILABLE_SPACE_MODE_DEFINITE    = 0,  // wrap at exactly availableWidth pixels
    LAYOUT_AVAILABLE_SPACE_MODE_MIN_CONTENT = 1,  // wrap at every possible break (narrowest)
    LAYOUT_AVAILABLE_SPACE_MODE_MAX_CONTENT = 2   // never wrap (widest)
} LayoutAvailableSpaceMode;

typedef struct LayoutTextMeasureResult {
    float width;
    float height;
} LayoutTextMeasureResult;

// Called by the layout engine for every node that has no element children. Write the size the
// node's text needs into *pResult and return 1, or return 0 if the node has no text.
// (int32_t, not bool: feav's main.h defines its own 4-byte `bool`.)
// widthMode is a LayoutAvailableSpaceMode; availableWidth only matters for the DEFINITE mode.
typedef int32_t (*PFN_LayoutMeasureText)(void *pUserData, const DOBJPool *pPool, uint32_t itemIndex,
                                      int32_t widthMode, float availableWidth,
                                      LayoutTextMeasureResult *pResult);

// ---- exported by the Rust library ----

// Lays out every node under rootGlobalId.
//   pResults        = the flat ComputedElement array (app->caches.results), indexed by global id.
//   resultCapacity  = how many ComputedElements of it are committed:
//                     app->caches.result_committed_size / sizeof(ComputedElement).
// The window size is read from pResults[rootGlobalId].width / .height. Writes x, y (window
// coordinates), width, height into pResults[global id] for every node under the root. Style arrays are
// never modified, so it can be called again after a resize or a style change. Stops the program with a
// message if pResults is NULL, misaligned, or too small for a node (instead of corrupting memory).
extern void feav_layout_compute(DOBJPool *pPool, ComputedElement *pResults, uint64_t resultCapacity, uint64_t rootGlobalId);

// Registers the text measure callback (NULL unregisters: every leaf then measures 0 x 0).
extern void feav_layout_set_text_measure(PFN_LayoutMeasureText pfnMeasureText, void *pUserData);

size_t feav_layout_pool_size(void);
size_t feav_layout_computed_element_size(void);
// Debug: prints every occupied node with its style (from the pool) and layout (from pResults).
extern void feav_layout_dump(const DOBJPool *pPool, const ComputedElement *pResults);

// ---- optional helper: monospace text metrics (replace with real font metrics later) ----
// Greedy word wrap of pText (words split by spaces, '\n' forces a line break).
static inline void layoutMeasureMonospaceText(const char *pText, uint32_t textLength, int32_t widthMode,
                                              float availableWidth, float characterWidth, float lineHeight,
                                              LayoutTextMeasureResult *pResult)
{
    uint32_t longestWordCharacterCount = 0;
    uint32_t wordCharacterCount = 0;
    for (uint32_t characterIndex = 0; characterIndex <= textLength; characterIndex++) {
        int32_t isSeparator = characterIndex == textLength || pText[characterIndex] == ' ' || pText[characterIndex] == '\n';
        if (isSeparator) {
            if (wordCharacterCount > longestWordCharacterCount) longestWordCharacterCount = wordCharacterCount;
            wordCharacterCount = 0;
        } else {
            wordCharacterCount++;
        }
    }

    uint32_t wrapCharacterCount = UINT32_MAX;  // MAX_CONTENT: never wrap
    if (widthMode == LAYOUT_AVAILABLE_SPACE_MODE_DEFINITE) {
        wrapCharacterCount = (uint32_t)(availableWidth / characterWidth);
    } else if (widthMode == LAYOUT_AVAILABLE_SPACE_MODE_MIN_CONTENT) {
        wrapCharacterCount = longestWordCharacterCount;
    }

    uint32_t lineCount = 0;
    uint32_t lineCharacterCount = 0;
    uint32_t widestLineCharacterCount = 0;
    int32_t lineHasContent = 0;
    wordCharacterCount = 0;
    for (uint32_t characterIndex = 0; characterIndex <= textLength; characterIndex++) {
        char character = characterIndex == textLength ? ' ' : pText[characterIndex];
        if (character != ' ' && character != '\n') {
            wordCharacterCount++;
            continue;
        }
        if (wordCharacterCount > 0) {
            if (!lineHasContent) {
                lineCount++;
                lineCharacterCount = wordCharacterCount;
                lineHasContent = 1;
            } else if (lineCharacterCount + 1 + wordCharacterCount <= wrapCharacterCount) {
                lineCharacterCount += 1 + wordCharacterCount;
            } else {
                if (lineCharacterCount > widestLineCharacterCount) widestLineCharacterCount = lineCharacterCount;
                lineCount++;
                lineCharacterCount = wordCharacterCount;
            }
            wordCharacterCount = 0;
        }
        if (character == '\n') {
            if (lineCharacterCount > widestLineCharacterCount) widestLineCharacterCount = lineCharacterCount;
            lineHasContent = 0;
            lineCharacterCount = 0;
        }
    }
    if (lineCharacterCount > widestLineCharacterCount) widestLineCharacterCount = lineCharacterCount;
    pResult->width = (float)widestLineCharacterCount * characterWidth;
    pResult->height = (float)lineCount * lineHeight;
}

#endif