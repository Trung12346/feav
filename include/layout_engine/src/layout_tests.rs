// Tests for the whole Taffy integration. Each test builds a small tree in a hand-made pool
// (slots in pre-order, see pool_tree.rs), runs `feav_layout_compute`, and checks the boxes.
// Only compiled by `cargo test`; not part of the library.

use core::ffi::c_void;
use std::alloc::{alloc_zeroed, dealloc, Layout};

use crate::display::LayoutDisplay;
use crate::pool_layout::compute_layout;
use crate::{ComputedElement, POOL_ELEMENT_COUNT};
use crate::text_measure::{feav_layout_set_text_measure, LayoutAvailableSpaceMode, LayoutTextMeasureResult};
use crate::{
    DOBJPool, LayoutBox, Unit, UNIT_OFFSET_BORDER_BOTTOM, UNIT_OFFSET_BORDER_LEFT, UNIT_OFFSET_BORDER_RIGHT,
    UNIT_OFFSET_BORDER_TOP, UNIT_OFFSET_DISPLAY, UNIT_OFFSET_HEIGHT, UNIT_OFFSET_INSET_BOTTOM, UNIT_OFFSET_INSET_LEFT,
    UNIT_OFFSET_INSET_RIGHT, UNIT_OFFSET_INSET_TOP, UNIT_OFFSET_MARGIN_BOTTOM, UNIT_OFFSET_MARGIN_LEFT,
    UNIT_OFFSET_MARGIN_RIGHT, UNIT_OFFSET_MARGIN_TOP, UNIT_OFFSET_PADDING_BOTTOM, UNIT_OFFSET_PADDING_LEFT,
    UNIT_OFFSET_PADDING_RIGHT, UNIT_OFFSET_PADDING_TOP, UNIT_OFFSET_WIDTH,
};

const ALL_MARGIN_UNIT_OFFSETS: [u32; 4] =
    [UNIT_OFFSET_MARGIN_TOP, UNIT_OFFSET_MARGIN_BOTTOM, UNIT_OFFSET_MARGIN_LEFT, UNIT_OFFSET_MARGIN_RIGHT];
const ALL_PADDING_UNIT_OFFSETS: [u32; 4] =
    [UNIT_OFFSET_PADDING_TOP, UNIT_OFFSET_PADDING_BOTTOM, UNIT_OFFSET_PADDING_LEFT, UNIT_OFFSET_PADDING_RIGHT];
const ALL_BORDER_UNIT_OFFSETS: [u32; 4] =
    [UNIT_OFFSET_BORDER_TOP, UNIT_OFFSET_BORDER_BOTTOM, UNIT_OFFSET_BORDER_LEFT, UNIT_OFFSET_BORDER_RIGHT];

/// Owns one zeroed pool and its results array for the duration of a test.
struct TestPool {
    p_pool: *mut DOBJPool,
    /// `results[global id]`, like `AppCaches.results` in C. A `Vec` is a growable array; because
    /// `ComputedElement` is 8-byte aligned, its memory is 8-byte aligned too.
    results: Vec<ComputedElement>,
}

// `Drop` is a trait whose `drop` function runs automatically when the value goes out of
// scope, like a destructor. Here it frees the memory.
impl Drop for TestPool {
    fn drop(&mut self) {
        unsafe { dealloc(self.p_pool as *mut u8, Layout::new::<DOBJPool>()) };
    }
}

impl TestPool {
    /// A pool with room for one pool of results, whose root (slot 0) has the window size 800 x 600
    /// stored in `results[0]`, where the parser stores it.
    fn new() -> TestPool {
        TestPool::with_pool_count(1)
    }

    /// Same, with room in the results array for `pool_count` pools.
    // `vec![value; n]` builds a Vec holding n copies of value.
    fn with_pool_count(pool_count: usize) -> TestPool {
        let p_pool = unsafe { alloc_zeroed(Layout::new::<DOBJPool>()) } as *mut DOBJPool;
        let mut test_pool =
            TestPool { p_pool, results: vec![ComputedElement::default(); POOL_ELEMENT_COUNT * pool_count] };
        test_pool.set_window(800.0, 600.0);
        test_pool
    }

    fn set_window(&mut self, width: f32, height: f32) {
        self.results[0].width = width;
        self.results[0].height = height;
    }

    fn pool(&mut self) -> &mut DOBJPool {
        unsafe { &mut *self.p_pool }
    }

    /// Declare that `parent` has these `children` (slots), linking them as siblings.
    fn set_children(&mut self, parent: usize, children: &[usize]) {
        let p_pool = self.p_pool;
        let pool = self.pool();
        pool.child_count[parent] = children.len() as u8;
        // `.windows(2)` visits every neighbouring pair: (first, second), (second, third), ...
        for pair in children.windows(2) {
            pool.pool_next_sib[pair[0]] = p_pool;
            pool.idx_next_sib[pair[0]] = pair[1] as u16;
        }
    }

    /// Write one 3-bit code into the node's 64-bit flags (`u64` suffix = 64-bit number).
    fn set_flag_code(&mut self, item: usize, offset: u32, code: u64) {
        let pool = self.pool();
        pool.prty_flags[item] = (pool.prty_flags[item] & !(0b111u64 << offset)) | (code << offset);
    }

    fn set_unit(&mut self, item: usize, unit_offset: u32, unit: Unit) {
        self.set_flag_code(item, unit_offset, unit as u64);
    }

    /// Give all four sides of a margin / padding / border / inset the same unit.
    fn set_units_of_all_sides(&mut self, item: usize, offsets: [u32; 4], unit: Unit) {
        for offset in offsets {
            self.set_unit(item, offset, unit);
        }
    }

    fn set_display(&mut self, item: usize, display: LayoutDisplay) {
        self.set_flag_code(item, UNIT_OFFSET_DISPLAY, display as u64);
    }

    fn set_width(&mut self, item: usize, unit: Unit, value: f32) {
        self.set_unit(item, UNIT_OFFSET_WIDTH, unit);
        self.pool().width[item] = value;
    }

    fn set_height(&mut self, item: usize, unit: Unit, value: f32) {
        self.set_unit(item, UNIT_OFFSET_HEIGHT, unit);
        self.pool().height[item] = value;
    }

    fn set_tag(&mut self, item: usize, tag: &str) {
        self.pool().tag_names[item][..tag.len()].copy_from_slice(tag.as_bytes());
    }

    fn compute(&mut self) {
        unsafe { compute_layout(self.p_pool, self.results.as_mut_ptr(), self.results.len() as u64, 0) };
    }

    /// (x, y, width, height) of one node after layout, by global id.
    fn rectangle(&self, global_id: usize) -> (f32, f32, f32, f32) {
        let result = &self.results[global_id];
        (result.x, result.y, result.width, result.height)
    }
}

#[test]
fn block_children_stack_and_positions_are_absolute() {
    // 0 root -- 1 A (h 50) , 2 B -- 3 C (h 10)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 2]);
    t.set_children(2, &[3]);
    t.set_height(1, Unit::Pixel, 50.0);
    t.set_height(3, Unit::Pixel, 10.0);
    t.compute();
    assert_eq!(t.rectangle(0), (0.0, 0.0, 800.0, 600.0)); // root = window
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 50.0));
    assert_eq!(t.rectangle(2), (0.0, 50.0, 800.0, 10.0));
    assert_eq!(t.rectangle(3), (0.0, 50.0, 800.0, 10.0)); // absolute y = B's 50
}

#[test]
fn layout_can_run_again_with_the_same_result() {
    let mut t = TestPool::new();
    t.set_children(0, &[1, 2]);
    t.set_height(1, Unit::Pixel, 50.0);
    t.set_width(2, Unit::Pixel, 100.0);
    t.compute();
    let first = (t.rectangle(0), t.rectangle(1), t.rectangle(2));
    t.compute();
    assert_eq!(first, (t.rectangle(0), t.rectangle(1), t.rectangle(2)));
    // style inputs are untouched
    assert_eq!(t.pool().height[1], 50.0);
    assert_eq!(t.pool().width[2], 100.0);
}

#[test]
fn display_none_takes_no_space_and_hides_descendants() {
    // 0 root -- 1 head (tag, hidden by UA default) -- 2 child ; 3 A (h 50)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 3]);
    t.set_children(1, &[2]);
    t.set_tag(1, "head");
    t.set_height(1, Unit::Pixel, 999.0);
    t.set_height(2, Unit::Pixel, 999.0);
    t.set_height(3, Unit::Pixel, 50.0);
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 0.0, 0.0));
    assert_eq!(t.rectangle(2), (0.0, 0.0, 0.0, 0.0));
    assert_eq!(t.rectangle(3), (0.0, 0.0, 800.0, 50.0)); // A starts at the top
}

#[test]
fn explicit_display_overrides_the_tag_default() {
    // <head> forced to block by the document's own CSS
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_tag(1, "head");
    t.set_display(1, LayoutDisplay::Block);
    t.set_height(1, Unit::Pixel, 40.0);
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 40.0));
}

#[test]
fn flex_row_places_children_side_by_side() {
    // 0 root -- 1 F (display: flex) -- 2 C (100x50), 3 D (200x30)
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_children(1, &[2, 3]);
    t.set_display(1, LayoutDisplay::Flex);
    t.set_width(2, Unit::Pixel, 100.0);
    t.set_height(2, Unit::Pixel, 50.0);
    t.set_width(3, Unit::Pixel, 200.0);
    t.set_height(3, Unit::Pixel, 30.0);
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 50.0)); // as tall as the tallest child
    assert_eq!(t.rectangle(2), (0.0, 0.0, 100.0, 50.0));
    assert_eq!(t.rectangle(3), (100.0, 0.0, 200.0, 30.0));
}

#[test]
fn percent_and_viewport_units_resolve() {
    // 1: width 50% (of 800), 2: width 10vw (80), 3: height 10vh (60), 4: height 10vmin (60)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 2, 3, 4]);
    t.set_width(1, Unit::Relative, 50.0);
    t.set_height(1, Unit::Pixel, 1.0);
    t.set_width(2, Unit::VWidth, 10.0);
    t.set_height(2, Unit::Pixel, 1.0);
    t.set_height(3, Unit::VHeight, 10.0);
    t.set_height(4, Unit::VMin, 10.0);
    t.compute();
    assert_eq!(t.rectangle(1).2, 400.0);
    assert_eq!(t.rectangle(2).2, 80.0);
    assert_eq!(t.rectangle(3).3, 60.0);
    assert_eq!(t.rectangle(4).3, 60.0);
}

#[test]
fn padding_and_margin_move_and_shrink_boxes() {
    // 0 root -- 1 P (padding 5) -- 2 C (h 10) ; 3 M (margin 10, h 30)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 3]);
    t.set_children(1, &[2]);
    t.set_units_of_all_sides(1, ALL_PADDING_UNIT_OFFSETS, Unit::Pixel);
    t.pool().padding[1] = LayoutBox { top: 5.0, bottom: 5.0, left: 5.0, right: 5.0 };
    t.set_height(2, Unit::Pixel, 10.0);
    t.set_units_of_all_sides(3, ALL_MARGIN_UNIT_OFFSETS, Unit::Pixel);
    t.pool().margin[3] = LayoutBox { top: 10.0, bottom: 10.0, left: 10.0, right: 10.0 };
    t.set_height(3, Unit::Pixel, 30.0);
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 20.0)); // 10 content + 2 * 5 padding
    assert_eq!(t.rectangle(2), (5.0, 5.0, 790.0, 10.0));
    assert_eq!(t.rectangle(3), (10.0, 30.0, 780.0, 30.0)); // 20 (P) + 10 margin top
}

#[test]
fn absolute_child_is_placed_from_its_positioned_ancestor() {
    // 0 root -- 1 A (h 50) ; 2 B (relative) -- 3 C (absolute, top 5, left 7, 10x10)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 2]);
    t.set_children(2, &[3]);
    t.set_height(1, Unit::Pixel, 50.0);
    t.pool().position[2] = 1; // relative
    t.pool().position[3] = 2; // absolute
    t.set_width(3, Unit::Pixel, 10.0);
    t.set_height(3, Unit::Pixel, 10.0);
    t.set_unit(3, UNIT_OFFSET_INSET_TOP, Unit::Pixel);
    t.set_unit(3, UNIT_OFFSET_INSET_LEFT, Unit::Pixel);
    t.pool().inset[3] = LayoutBox { top: 5.0, bottom: 0.0, left: 7.0, right: 0.0 };
    t.compute();
    assert_eq!(t.rectangle(2), (0.0, 50.0, 800.0, 0.0));
    assert_eq!(t.rectangle(3), (7.0, 55.0, 10.0, 10.0));
}

#[test]
fn absolute_child_inside_a_static_box_uses_the_positioned_ancestor() {
    // 0 root -- 1 B (relative) -- 2 S (h 20) , 3 D (static) -- 4 C (absolute top 5, left 7)
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_children(1, &[2, 3]);
    t.set_children(3, &[4]);
    t.pool().position[1] = 1;
    t.set_height(2, Unit::Pixel, 20.0);
    t.pool().position[4] = 2;
    t.set_width(4, Unit::Pixel, 10.0);
    t.set_height(4, Unit::Pixel, 10.0);
    t.set_unit(4, UNIT_OFFSET_INSET_TOP, Unit::Pixel);
    t.set_unit(4, UNIT_OFFSET_INSET_LEFT, Unit::Pixel);
    t.pool().inset[4] = LayoutBox { top: 5.0, bottom: 0.0, left: 7.0, right: 0.0 };
    t.compute();
    assert_eq!(t.rectangle(3).1, 20.0); // D sits below S
    // C is measured from B (the positioned ancestor at y 0), NOT from D (y 20)
    assert_eq!(t.rectangle(4), (7.0, 5.0, 10.0, 10.0));
}

#[test]
fn fixed_box_is_placed_from_the_window_even_when_nested() {
    // 0 root -- 1 A (h 50) ; 2 B -- 3 F (fixed, top 5, left 7, 10x10)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 2]);
    t.set_children(2, &[3]);
    t.set_height(1, Unit::Pixel, 50.0);
    t.pool().position[3] = 3; // fixed
    t.set_width(3, Unit::Pixel, 10.0);
    t.set_height(3, Unit::Pixel, 10.0);
    t.set_unit(3, UNIT_OFFSET_INSET_TOP, Unit::Pixel);
    t.set_unit(3, UNIT_OFFSET_INSET_LEFT, Unit::Pixel);
    t.pool().inset[3] = LayoutBox { top: 5.0, bottom: 0.0, left: 7.0, right: 0.0 };
    t.compute();
    assert_eq!(t.rectangle(2).1, 50.0); // B is below A
    assert_eq!(t.rectangle(3), (7.0, 5.0, 10.0, 10.0)); // F ignores B and A: window-relative
}

#[test]
fn right_and_bottom_insets_anchor_absolute_boxes() {
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.pool().position[1] = 3; // fixed: relative to the window
    t.set_width(1, Unit::Pixel, 100.0);
    t.set_height(1, Unit::Pixel, 50.0);
    t.set_unit(1, UNIT_OFFSET_INSET_RIGHT, Unit::Pixel);
    t.set_unit(1, UNIT_OFFSET_INSET_BOTTOM, Unit::Pixel);
    t.pool().inset[1] = LayoutBox { top: 0.0, bottom: 10.0, left: 0.0, right: 20.0 };
    t.compute();
    assert_eq!(t.rectangle(1), (680.0, 540.0, 100.0, 50.0)); // 800-20-100, 600-10-50
}

#[test]
fn each_side_of_padding_can_have_its_own_unit() {
    // 0 root -- 1 P (padding: left 10px, right 5%, top/bottom unset) -- 2 C (h 10)
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_children(1, &[2]);
    t.set_unit(1, UNIT_OFFSET_PADDING_LEFT, Unit::Pixel);
    t.set_unit(1, UNIT_OFFSET_PADDING_RIGHT, Unit::Relative);
    t.pool().padding[1] = LayoutBox { top: 0.0, bottom: 0.0, left: 10.0, right: 5.0 }; // 5 (%) of 800 = 40
    t.set_height(2, Unit::Pixel, 10.0);
    t.compute();
    assert_eq!(t.rectangle(2), (10.0, 0.0, 750.0, 10.0)); // 800 - 10 - 40
}

#[test]
fn border_widths_are_read_and_take_space() {
    // 0 root -- 1 B (border 3px on all sides) -- 2 C (h 10)
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_children(1, &[2]);
    t.set_units_of_all_sides(1, ALL_BORDER_UNIT_OFFSETS, Unit::Pixel);
    t.pool().border[1] = LayoutBox { top: 3.0, bottom: 3.0, left: 3.0, right: 3.0 };
    t.set_height(2, Unit::Pixel, 10.0);
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 16.0)); // 10 content + 2 * 3 border
    assert_eq!(t.rectangle(2), (3.0, 3.0, 794.0, 10.0));
}

#[test]
fn display_code_lives_beside_the_units_without_disturbing_them() {
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_width(1, Unit::Pixel, 100.0);
    t.set_display(1, LayoutDisplay::Flex);
    // the width unit is still Pixel and the display code is still Flex
    let flags = t.pool().prty_flags[1];
    assert_eq!(flags & 0b111, Unit::Pixel as u64);
    assert_eq!((flags >> UNIT_OFFSET_DISPLAY) & 0b111, LayoutDisplay::Flex as u64);
    t.compute();
    assert_eq!(t.rectangle(1).2, 100.0);
}

#[test]
fn layout_writes_only_the_four_geometry_fields_and_keeps_the_colour_channels() {
    let mut t = TestPool::new();
    t.set_children(0, &[1]);
    t.set_height(1, Unit::Pixel, 50.0);
    t.results[1].color_channels = [11, 22, 33, 44]; // written by the parser in the real app
    t.results[0].color_channels = [5, 6, 7, 8];
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 50.0));
    assert_eq!(t.results[1].color_channels, [11, 22, 33, 44]);
    assert_eq!(t.results[0].color_channels, [5, 6, 7, 8]);
}

#[test]
fn the_root_keeps_the_window_size_it_started_with() {
    let mut t = TestPool::new();
    t.set_window(1000.0, 760.0);
    t.compute();
    assert_eq!(t.rectangle(0), (0.0, 0.0, 1000.0, 760.0));
    // a resize: change the root's size, run again
    t.set_window(500.0, 300.0);
    t.compute();
    assert_eq!(t.rectangle(0), (0.0, 0.0, 500.0, 300.0));
}

#[test]
fn nodes_in_the_second_pool_use_the_second_block_of_results() {
    // 0 root (pool 0) -- 1 A (h 50) ; 1023 B (last slot of pool 0) -- 1024 C (slot 0 of pool 1, h 10)
    let mut t = TestPool::with_pool_count(2);
    let p0 = t.p_pool;
    let p1 = unsafe { alloc_zeroed(Layout::new::<DOBJPool>()) } as *mut DOBJPool;
    unsafe {
        (*p0).p_next = p1;
        (*p1).p_prev = p0;
        (*p1).pool_index = 1;
    }
    t.set_children(0, &[1, 1023]);
    t.pool().child_count[1023] = 1; // C is the next slot: slot 0 of pool 1
    t.set_height(1, Unit::Pixel, 50.0);
    unsafe {
        (*p1).prty_flags[0] = 1 << UNIT_OFFSET_HEIGHT;
        (*p1).height[0] = 10.0;
    }
    t.compute();
    assert_eq!(t.rectangle(1023), (0.0, 50.0, 800.0, 10.0)); // B wraps C
    assert_eq!(t.rectangle(1024), (0.0, 50.0, 800.0, 10.0)); // C = results[1024], absolute y 50
    unsafe { dealloc(p1 as *mut u8, Layout::new::<DOBJPool>()) };
}

#[test]
#[should_panic(expected = "no result slot")]
fn a_node_without_committed_result_memory_fails_loudly() {
    // C = global id 1024 needs results[1024], but only one pool of results exists (1024 elements)
    let mut t = TestPool::new();
    let p0 = t.p_pool;
    let p1 = unsafe { alloc_zeroed(Layout::new::<DOBJPool>()) } as *mut DOBJPool;
    unsafe {
        (*p0).p_next = p1;
        (*p1).p_prev = p0;
        (*p1).pool_index = 1;
    }
    t.set_children(0, &[1, 1023]); // B = slot 1023, its first child is the next slot: pool 1, slot 0
    t.pool().child_count[1023] = 1;
    t.compute();
}

// ---- text measurement through the registered C-style callback ----

// A fake text: only nodes tagged "p" have text. It is 100 px wide on one line, 20 px tall per
// line, and wraps when the available width is smaller.
unsafe extern "C" fn fake_text_measure(
    _p_user_data: *mut c_void,
    p_pool: *const DOBJPool,
    item_index: u32,
    width_mode: i32,
    available_width: f32,
    p_result: *mut LayoutTextMeasureResult,
) -> i32 {
    if (*p_pool).tag_names[item_index as usize][0] != b'p' {
        return 0;
    }
    let (width, lines) = if width_mode == LayoutAvailableSpaceMode::Definite as i32 && available_width < 100.0 {
        (available_width, (100.0 / available_width).ceil())
    } else {
        (100.0, 1.0)
    };
    *p_result = LayoutTextMeasureResult { width, height: 20.0 * lines };
    1
}

#[test]
fn text_leaves_are_measured_by_the_registered_callback() {
    // 0 root -- 1 P1 (text) ; 2 W (width 50) -- 3 P2 (text, wraps into 2 lines)
    let mut t = TestPool::new();
    t.set_children(0, &[1, 2]);
    t.set_children(2, &[3]);
    t.set_tag(1, "p");
    t.set_tag(3, "p");
    t.set_width(2, Unit::Pixel, 50.0);
    unsafe { feav_layout_set_text_measure(Some(fake_text_measure), core::ptr::null_mut()) };
    t.compute();
    assert_eq!(t.rectangle(1), (0.0, 0.0, 800.0, 20.0));
    assert_eq!(t.rectangle(2), (0.0, 20.0, 50.0, 40.0));
    assert_eq!(t.rectangle(3), (0.0, 20.0, 50.0, 40.0));
}

// ---- a document shaped like feav's index.html (same 104 nodes, same explicit sizes) ----

/// `PARENT_OF_NODE[n]` = parent slot of node n, copied from feav's own dump of index.html.
const PARENT_OF_NODE: [i32; 104] = [
    -1, 0, 1, 1, 1, 1, 0, 6, 7, 8, 8, 10, 10, 10, 10, 6, 15, 16, 16, 16, 15, 20, 20, 22, 22, 22, 25, 25, 25, 25, 20, 30, 30,
    30, 33, 33, 33, 33, 33, 20, 39, 39, 15, 42, 42, 44, 45, 46, 46, 46, 46, 44, 51, 52, 52, 52, 52, 51, 57, 57, 57, 57, 51,
    62, 62, 62, 62, 15, 67, 67, 69, 69, 69, 69, 69, 69, 75, 75, 75, 75, 69, 69, 69, 69, 15, 84, 85, 86, 87, 87, 87, 87, 15,
    92, 92, 92, 6, 96, 97, 97, 97, 97, 97, 6,
];

#[test]
fn index_html_shaped_document_lays_out_as_block() {
    let mut t = TestPool::new();
    // window 1000 x 760, as in feav's dump
    t.set_window(1000.0, 760.0);
    // children of every node, in slot order (= document order)
    for parent in 0..104usize {
        let children: Vec<usize> = (0..104usize).filter(|&node| PARENT_OF_NODE[node] == parent as i32).collect();
        if !children.is_empty() {
            t.set_children(parent, &children);
        }
    }
    t.set_tag(1, "head");
    t.set_tag(103, "script");
    t.set_width(1, Unit::Pixel, 100.0); // <head style="width: 100">
    t.set_width(6, Unit::Pixel, 1000.0); // <body style="width: 1000">
    t.set_height(7, Unit::Pixel, 300.0); // header
    t.set_height(16, Unit::Pixel, 500.0); // hero section
    t.set_height(20, Unit::Pixel, 300.0); // features section
    t.set_height(30, Unit::Pixel, 100.0); // article 2
    t.set_height(39, Unit::Pixel, 100.0); // article 3
    t.set_height(42, Unit::Pixel, 700.0); // table section
    t.compute();

    assert_eq!(t.rectangle(0), (0.0, 0.0, 1000.0, 760.0)); // html = window
    assert_eq!(t.rectangle(1), (0.0, 0.0, 0.0, 0.0)); // head: display none
    assert_eq!(t.rectangle(7), (0.0, 0.0, 1000.0, 300.0)); // header
    assert_eq!(t.rectangle(15), (0.0, 300.0, 1000.0, 1500.0)); // main = 500 + 300 + 700
    assert_eq!(t.rectangle(16), (0.0, 300.0, 1000.0, 500.0)); // hero
    assert_eq!(t.rectangle(20), (0.0, 800.0, 1000.0, 300.0)); // features
    assert_eq!(t.rectangle(30), (0.0, 800.0, 1000.0, 100.0)); // article 2
    assert_eq!(t.rectangle(39), (0.0, 900.0, 1000.0, 100.0)); // article 3
    assert_eq!(t.rectangle(42), (0.0, 1100.0, 1000.0, 700.0)); // table section
    assert_eq!(t.rectangle(96), (0.0, 1800.0, 1000.0, 0.0)); // footer
    assert_eq!(t.rectangle(103), (0.0, 0.0, 0.0, 0.0)); // script: display none
    assert_eq!(t.rectangle(6), (0.0, 0.0, 1000.0, 1800.0)); // body = 300 + 1500
}
