// =====================================================================================
// display.rs  --  which layout algorithm does a node get?
// =====================================================================================
//
// Terms:
//  * "display"     = the CSS property that says how a box lays out its children
//                    (block, flex, none, ...).
//  * "UA default"  = what a browser's built-in stylesheet says when the page's own CSS says
//                    nothing. Example: <head> and <script> are `display: none` by default.
//  * "layout mode" = the algorithm feav actually runs for a node. Taffy offers block, flex
//                    and grid. Inline text flow and tables are NOT part of Taffy.

use crate::{DOBJPool, UNIT_OFFSET_DISPLAY};

/// The 3-bit display code stored in `prty_flags` at `UNIT_OFFSET_DISPLAY`. The numbers are
/// the C `DOBJ_PROPERTY_UNIT_DISPLAY_*_BIT` codes.
/// `Unset` (0) is what a zero-filled pool contains: "the document said nothing, use the
/// UA default for this tag".
// `#[repr(u8)]` = store this enum as one byte whose values are the numbers written below,
// so the numbers match the C enum. `derive(...)` asks the compiler to write standard code
// for copying (`Clone, Copy`), printing (`Debug`) and comparing with `==` (`PartialEq, Eq`).
#[repr(u8)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LayoutDisplay {
    Unset = 0,
    Block = 1,
    None = 2,
    Inline = 3,
    InlineBlock = 4,
    Flex = 5,
    Grid = 6,
}

impl LayoutDisplay {
    /// Turn the stored 3-bit code back into the enum. Unknown codes count as `Unset`.
    pub fn from_stored_code(stored_code: u64) -> LayoutDisplay {
        match stored_code {
            1 => LayoutDisplay::Block,
            2 => LayoutDisplay::None,
            3 => LayoutDisplay::Inline,
            4 => LayoutDisplay::InlineBlock,
            5 => LayoutDisplay::Flex,
            6 => LayoutDisplay::Grid,
            _ => LayoutDisplay::Unset,
        }
    }
}

/// The algorithm feav runs for one node.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LayoutMode {
    /// `display: none`: no box, no space, descendants hidden too.
    Hidden,
    /// Block layout: children stacked top to bottom.
    Block,
    /// Flexbox layout.
    Flex,
}

/// Tags a browser hides by default (`display: none` in the UA stylesheet).
const TAGS_HIDDEN_BY_DEFAULT: [&str; 14] = [
    "head", "meta", "title", "style", "script", "link", "base", "template", "noscript", "area", "datalist",
    "param", "source", "track",
];

/// The UA default display for a tag name. Everything not listed is a block box.
fn default_display_for_tag(tag_name: &[u8]) -> LayoutDisplay {
    // `for x in array` visits each element. `&str` = a borrowed piece of text.
    for hidden_tag in TAGS_HIDDEN_BY_DEFAULT {
        // `as_bytes()` views the text as raw bytes so it can be compared with `tag_name`.
        // `eq_ignore_ascii_case` = equal if you ignore upper/lower case letters.
        if tag_name.eq_ignore_ascii_case(hidden_tag.as_bytes()) {
            return LayoutDisplay::None;
        }
    }
    LayoutDisplay::Block
}

/// The display of one node: the document's own value if it has one, else the UA default.
pub fn display_of_node(pool: &DOBJPool, item_index: usize) -> LayoutDisplay {
    let stored_code = (pool.prty_flags[item_index] >> UNIT_OFFSET_DISPLAY) & 0b111;
    let stored = LayoutDisplay::from_stored_code(stored_code);
    if stored != LayoutDisplay::Unset {
        return stored;
    }
    // `tag_names[i]` is 16 bytes of text padded with 0 bytes. Cut it at the first 0.
    let tag_name_padded = &pool.tag_names[item_index];
    // `.iter().position(|b| *b == 0)` = index of the first byte that is 0, if any.
    // `|b| *b == 0` is a closure (small inline function) taking one byte reference `b`.
    // `.unwrap_or(16)` = "use that index, or 16 if there is none" (no zero byte found).
    let tag_name_length = tag_name_padded.iter().position(|byte| *byte == 0).unwrap_or(16);
    default_display_for_tag(&tag_name_padded[..tag_name_length])
}

/// Which algorithm runs for this node.
//
// Limits (on purpose, until the data exists):
//  * Inline / InlineBlock are laid out as blocks. Taffy has no inline text flow.
//  * Grid is laid out as a block. The pool has no grid properties yet.
pub fn layout_mode_of_node(pool: &DOBJPool, item_index: usize) -> LayoutMode {
    match display_of_node(pool, item_index) {
        LayoutDisplay::None => LayoutMode::Hidden,
        LayoutDisplay::Flex => LayoutMode::Flex,
        // `_` matches everything not listed above.
        _ => LayoutMode::Block,
    }
}
