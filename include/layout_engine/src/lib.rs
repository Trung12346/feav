#[no_mangle]
pub extern "C" fn feav_layout_ping() -> i32 { 42 }

// Rust mirror of feav's `DOBJPool` (dom/specs.h).
//
// Rules for keeping this honest:
//  * Field ORDER and TYPES must match the C struct exactly.
//  * Never construct or copy a DOBJPool in Rust (it is ~155 KB). Only ever
//    touch it through a raw pointer / reference that C gave us.
//  * Heap / BHeap pointers are opaque here: layout never reads them.

mod display;
mod pool_layout;
mod pool_style;
mod pool_tree;
mod text_measure;

#[cfg(test)]
mod layout_tests;

use core::ffi::c_void;
use core::mem::size_of;

pub const POOL_ELEMENT_COUNT: usize = 1024;
pub const STATUS_OCCUPIED: u8 = 0b0000_0001; // POOL_STATUS_IS_OCCUPIED_BIT

/// C: `poid` (pool object id)
pub type Poid = u16;
/// C: `htmltag_t`
pub type HtmlTag = u16;
/// C: `UINTx_T(MAX_CHILD_VAR_WIDTH)`. MAX_CHILD_VAR_WIDTH defaults to 8, so this
/// ASSUMES uint8_t. If your UINTx_T(8) is something else, change it here
/// (and the size/offset asserts below will tell you).
pub type ChildCount = u8;
/// C: `DOBJpropertyUnitFlag` (uint64_t): 3 bits per property, see the UNIT_OFFSET_* constants.
pub type UnitFlags = u64;
/// C: `CSSattributePosition` (uint8_t)
pub type Position = u8;

/// C: `LayoutBoxProperties`
#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct LayoutBox {
    pub top: f32,
    pub bottom: f32,
    pub left: f32,
    pub right: f32,
}

/// C: `struct DOBJPool`, structure-of-arrays, 1024 nodes per pool,
/// pools form a doubly linked list.
#[repr(C)]
pub struct DOBJPool {
    pub pool_index: u16,
    pub live_objects: u16,
    pub p_prev: *mut DOBJPool,
    pub p_next: *mut DOBJPool,
    pub status_flags: [u8; POOL_ELEMENT_COUNT],

    pub tag_names: [[u8; 16]; POOL_ELEMENT_COUNT],
    pub tag_ids: [HtmlTag; POOL_ELEMENT_COUNT],

    pub heap_h_attrs: [*mut c_void; POOL_ELEMENT_COUNT],
    pub a_h_attrs: [u16; POOL_ELEMENT_COUNT],
    pub s_h_attrs: [u16; POOL_ELEMENT_COUNT],

    pub heap_h_inner: [*mut c_void; POOL_ELEMENT_COUNT],
    pub a_h_inner: [u64; POOL_ELEMENT_COUNT],
    pub s_h_inner: [u64; POOL_ELEMENT_COUNT],

    pub pool_parent: [*mut DOBJPool; POOL_ELEMENT_COUNT],
    pub pool_first_child: [*mut DOBJPool; POOL_ELEMENT_COUNT],
    pub pool_next_sib: [*mut DOBJPool; POOL_ELEMENT_COUNT],
    pub pool_prev_sib: [*mut DOBJPool; POOL_ELEMENT_COUNT],
    pub idx_parent: [Poid; POOL_ELEMENT_COUNT],
    pub idx_first_child: [Poid; POOL_ELEMENT_COUNT],
    pub idx_next_sib: [Poid; POOL_ELEMENT_COUNT],
    pub idx_prev_sib: [Poid; POOL_ELEMENT_COUNT],

    pub child_count: [ChildCount; POOL_ELEMENT_COUNT],

    pub prty_flags: [UnitFlags; POOL_ELEMENT_COUNT],

    pub position: [Position; POOL_ELEMENT_COUNT],
    pub x: [f32; POOL_ELEMENT_COUNT],
    pub y: [f32; POOL_ELEMENT_COUNT],
    pub width: [f32; POOL_ELEMENT_COUNT],
    pub height: [f32; POOL_ELEMENT_COUNT],

    pub inset: [LayoutBox; POOL_ELEMENT_COUNT],
    pub padding: [LayoutBox; POOL_ELEMENT_COUNT],
    pub margin: [LayoutBox; POOL_ELEMENT_COUNT],
    pub border: [LayoutBox; POOL_ELEMENT_COUNT],

    // --- Layout OUTPUT, kept apart from the style INPUT arrays width[] / height[] ---
    /// OUTPUT. Layout result width. Kept apart from `width[]` (the style INPUT).
    pub computed_width: [f32; POOL_ELEMENT_COUNT],
    /// OUTPUT. Layout result height. Kept apart from `height[]` (the style INPUT).
    pub computed_height: [f32; POOL_ELEMENT_COUNT],
}



// Bit offsets of each property's 3-bit unit inside `prty_flags` (C: DOBJpropertyUnitOffsets).
pub const UNIT_OFFSET_WIDTH: u32 = 0;
pub const UNIT_OFFSET_HEIGHT: u32 = 3;
pub const UNIT_OFFSET_INSET_TOP: u32 = 6;
pub const UNIT_OFFSET_INSET_BOTTOM: u32 = 9;
pub const UNIT_OFFSET_INSET_LEFT: u32 = 12;
pub const UNIT_OFFSET_INSET_RIGHT: u32 = 15;
pub const UNIT_OFFSET_FONT: u32 = 18;
pub const UNIT_OFFSET_MARGIN_TOP: u32 = 21;
pub const UNIT_OFFSET_MARGIN_BOTTOM: u32 = 24;
pub const UNIT_OFFSET_MARGIN_LEFT: u32 = 27;
pub const UNIT_OFFSET_MARGIN_RIGHT: u32 = 30;
pub const UNIT_OFFSET_PADDING_TOP: u32 = 33;
pub const UNIT_OFFSET_PADDING_BOTTOM: u32 = 36;
pub const UNIT_OFFSET_PADDING_LEFT: u32 = 39;
pub const UNIT_OFFSET_PADDING_RIGHT: u32 = 42;
pub const UNIT_OFFSET_BORDER_TOP: u32 = 45;
pub const UNIT_OFFSET_BORDER_BOTTOM: u32 = 48;
pub const UNIT_OFFSET_BORDER_LEFT: u32 = 51;
pub const UNIT_OFFSET_BORDER_RIGHT: u32 = 54;
/// The display slot holds a `LayoutDisplay` code (see display.rs), not a unit.
pub const UNIT_OFFSET_DISPLAY: u32 = 57;

#[repr(u32)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Unit {
    Auto = 0,
    Pixel = 1,
    VWidth = 2,
    VHeight = 3,
    Relative = 4,
    FCalc = 5,
    VMin = 6,
    VMax = 7,
}

impl Unit {
    #[inline]
    pub fn from_flags(flags: UnitFlags, offset: u32) -> Unit {
        match (flags >> offset) & 0b111 {
            0 => Unit::Auto,
            1 => Unit::Pixel,
            2 => Unit::VWidth,
            3 => Unit::VHeight,
            4 => Unit::Relative,
            5 => Unit::FCalc,
            6 => Unit::VMin,
            _ => Unit::VMax,
        }
    }
}

/// Global node id = pool_index * POOL_ELEMENT_COUNT + item_index
#[inline]
pub fn global_id(pool_index: u16, item_index: usize) -> u64 {
    pool_index as u64 * POOL_ELEMENT_COUNT as u64 + item_index as u64
}

// ---------------------------------------------------------------------------
// Exports for C
// ---------------------------------------------------------------------------

/// C side: `assert(feav_layout_pool_size() == sizeof(DOBJPool));` at startup.
#[no_mangle]
pub extern "C" fn feav_layout_pool_size() -> usize {
    size_of::<DOBJPool>()
}

/// Debug: walk every pool in the linked list (from any pool) and print each
/// occupied node. Compare the output with what C sees for the same nodes.
#[no_mangle]
pub unsafe extern "C" fn feav_layout_dump(pool: *const DOBJPool) {
    if pool.is_null() {
        return;
    }
    let mut p = pool;
    while !(*p).p_prev.is_null() {
        p = (*p).p_prev;
    }
    while !p.is_null() {
        let pr = &*p;
        for i in 0..POOL_ELEMENT_COUNT {
            if pr.status_flags[i] & STATUS_OCCUPIED == 0 {
                continue;
            }
            let pp = pr.pool_parent[i];
            let parent: i64 = if pp.is_null() {
                -1
            } else {
                global_id((*pp).pool_index, pr.idx_parent[i] as usize) as i64
            };
            let f = pr.prty_flags[i];
            println!(
                "node {} parent {} tag {} children {} | style w {:?} {} h {:?} {} | layout x {} y {} w {} h {}",
                global_id(pr.pool_index, i),
                parent,
                pr.tag_ids[i],
                pr.child_count[i],
                Unit::from_flags(f, UNIT_OFFSET_WIDTH),
                pr.width[i],
                Unit::from_flags(f, UNIT_OFFSET_HEIGHT),
                pr.height[i],
                pr.x[i],
                pr.y[i],
                pr.computed_width[i],
                pr.computed_height[i],
            );
        }
        p = pr.p_next;
    }
}