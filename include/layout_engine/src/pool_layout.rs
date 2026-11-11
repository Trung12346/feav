// =====================================================================================
// pool_layout.rs  --  run Taffy's layout and write the results into feav's pool
// =====================================================================================
//
// Terms (earlier files define: global id, item index, pre-order, style, view, viewport):
//  * "layout"            = the RESULT for one node: x, y, width, height.
//  * "available space"   = how much room the root may use (the window).
//  * "relative position" = Taffy reports each node's x/y measured from its PARENT's
//                          top-left corner. feav stores x/y measured from the WINDOW's
//                          top-left corner ("absolute"). A last pass converts.
//  * "cache"             = remembered answers, so Taffy does not recompute the same
//                          question twice within one layout run.
//
// INPUT (read only):  the pool's style arrays (width, height, margin, padding, border, inset,
//                     position, prty_flags, tag_names) and, for the window size, the root's
//                     `results[root].width` / `.height`.
// OUTPUT (written):   `results[global id]` = { x, y, width, height } of every node under the root.
//                     The colour channels in the same struct belong to the parser and are never touched.
// The style arrays are never modified, so layout can run again and again (window resize,
// style change) and always starts from the same inputs.

use core::mem::align_of;
use std::collections::HashMap;

use taffy::{
    compute_block_layout, compute_cached_layout, compute_flexbox_layout, compute_hidden_layout, compute_leaf_layout,
    compute_oof_layout, compute_root_layout, AvailableSpace, Cache, CacheTree, Layout, LayoutBlockContainer,
    LayoutContainingBlock, LayoutFlexboxContainer, LayoutInput, LayoutOutput, LayoutPartialTree, NodeId, RunMode, Size,
    TraversePartialTree,
};

use crate::display::{layout_mode_of_node, LayoutMode};
use crate::pool_style::{LayoutViewport, PoolNodeStyleView};
use crate::pool_tree::{PoolChildIterator, PoolTree};
use crate::text_measure::measure_leaf_text;
use crate::{ComputedElement, DOBJPool};

// -------------------------------------------------------------------------------------
// THE TREE THAT CAN BE LAID OUT
// -------------------------------------------------------------------------------------

/// `PoolTree` can only be walked. This wraps it and adds what layout needs: the viewport,
/// caches, and the ability to write results back into the pool.
pub struct PoolLayoutTree {
    pool_tree: PoolTree,
    /// The flat results array, indexed by global id (C: `AppCaches.results`).
    p_results: *mut ComputedElement,
    /// How many `ComputedElement`s of that array are committed (usable). Indexing past it would
    /// touch memory that does not exist, so every access is checked against it.
    result_capacity: usize,
    viewport: LayoutViewport,
    // `HashMap<K, V>` = a lookup table (a dictionary) from keys of type K to values of type V.
    // Cache per node, found by global id.
    cache_per_node: HashMap<u64, Cache>,
    // Absolute-positioned boxes Taffy found, per containing node. Taffy requires us to
    // keep this list; feav itself does not read it.
    hoisted_children_per_node: HashMap<u64, Vec<NodeId>>,
}

impl PoolLayoutTree {
    /// `p_any_pool` is a `*mut` (writable raw pointer) because layout WRITES into the pool.
    pub fn new(
        p_any_pool: *mut DOBJPool,
        p_results: *mut ComputedElement,
        result_capacity: usize,
        viewport: LayoutViewport,
    ) -> PoolLayoutTree {
        PoolLayoutTree {
            pool_tree: PoolTree { p_any_pool: p_any_pool as *const DOBJPool },
            p_results,
            result_capacity,
            viewport,
            cache_per_node: HashMap::new(),
            hoisted_children_per_node: HashMap::new(),
        }
    }

    fn style_view(&self, node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.pool_tree.node_style_view(node_id, self.viewport)
    }

    /// The pool holding `node_id` and the node's slot inside it.
    fn locate(&self, node_id: NodeId) -> (*mut DOBJPool, usize) {
        let (pool_index, item_index) = PoolTree::split_node_id(node_id);
        // `as *mut DOBJPool` turns the read-only pointer into a writable one. This is
        // fine because the pool belongs to C and C passed it to us as writable.
        let p_pool = unsafe { self.pool_tree.find_pool(pool_index) } as *mut DOBJPool;
        (p_pool, item_index)
    }

    /// The result slot of one node: `results[global id]`.
    // `assert!(condition, "message")` stops the program with the message if the condition is false.
    // Here that turns "write outside the committed results memory" into a clear error instead of
    // a silent corruption or a crash somewhere else.
    fn result_slot(&self, node_id: NodeId) -> *mut ComputedElement {
        let global: u64 = node_id.into();
        assert!(
            (global as usize) < self.result_capacity,
            "layout: node {} has no result slot (result capacity is {} elements; was a pool added without committing results memory?)",
            global,
            self.result_capacity
        );
        // `.add(n)` moves a raw pointer forward by n ELEMENTS (here n * 32 bytes).
        unsafe { self.p_results.add(global as usize) }
    }

    /// Which algorithm runs for this node (block, flex, hidden).
    fn layout_mode_of(&self, node_id: NodeId) -> LayoutMode {
        let (p_pool, item_index) = self.locate(node_id);
        layout_mode_of_node(unsafe { &*p_pool }, item_index)
    }

    /// Find the cache of one node, creating an empty one the first time.
    // `-> &mut Cache` = hands back a writable borrow of a Cache that lives inside `self`.
    fn cache_for(&mut self, node_id: NodeId) -> &mut Cache {
        let key: u64 = node_id.into();
        // `.entry(key).or_insert_with(f)` returns the existing value, or calls `f` to
        // build a new one and stores it. `Cache::new` is `f` (a function passed by name).
        self.cache_per_node.entry(key).or_insert_with(Cache::new)
    }
}

// How to walk children: PoolTree's answers.
impl TraversePartialTree for PoolLayoutTree {
    type ChildIter<'a>
        = PoolChildIterator<'a>
    where
        Self: 'a;

    fn child_ids(&self, parent_node_id: NodeId) -> Self::ChildIter<'_> {
        self.pool_tree.child_ids(parent_node_id)
    }
    fn child_count(&self, parent_node_id: NodeId) -> usize {
        self.pool_tree.child_count(parent_node_id)
    }
    fn get_child_id(&self, parent_node_id: NodeId, child_index: usize) -> NodeId {
        self.pool_tree.get_child_id(parent_node_id, child_index)
    }
}

// `LayoutPartialTree` = "a tree Taffy can run layout on".
impl LayoutPartialTree for PoolLayoutTree {
    type CustomIdent = String;

    type CoreContainerStyle<'a>
        = PoolNodeStyleView<'a>
    where
        Self: 'a;

    fn get_core_container_style(&self, node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.style_view(node_id)
    }

    // Taffy says: "node N ended up here with this size". We store it in the results array.
    fn set_unrounded_layout(&mut self, node_id: NodeId, layout: &Layout) {
        let p_result = self.result_slot(node_id);
        unsafe {
            // x and y hold the RELATIVE position for now; `make_positions_absolute`
            // converts them after layout. Only these four fields are written: the colour
            // channels next to them belong to the parser.
            (*p_result).x = layout.location.x;
            (*p_result).y = layout.location.y;
            (*p_result).width = layout.size.width;
            (*p_result).height = layout.size.height;
        }
    }

    // Taffy says: "compute the size/layout of this node". This is where the layout
    // algorithm is CHOSEN, per node.
    fn compute_child_layout(&mut self, node_id: NodeId, inputs: LayoutInput) -> LayoutOutput {
        // `compute_cached_layout(tree, node, inputs, closure)` first checks the cache and
        // only calls the CLOSURE when the answer is not cached. A closure is a small
        // anonymous function written inline: `|a, b, c| { body }`.
        compute_cached_layout(self, node_id, inputs, |tree, node_id, inputs| {
            let mode = tree.layout_mode_of(node_id);
            // Children of a hidden node are told to run in "hidden layout" mode by Taffy
            // itself (`RunMode::PerformHiddenLayout`), whatever their own display says.
            let output_is_hidden = mode == LayoutMode::Hidden || inputs.run_mode == RunMode::PerformHiddenLayout;
            let mut output = if output_is_hidden {
                // display: none -> zero-size box, descendants hidden too.
                compute_hidden_layout(tree, node_id)
            } else if tree.pool_tree.child_count(node_id) == 0 {
                // A LEAF (no element children): its size comes from its own style, or
                // from its text, which C measures through the registered callback.
                let (p_pool, item_index) = tree.locate(node_id);
                let style = tree.style_view(node_id);
                compute_leaf_layout(
                    inputs,
                    &style,
                    // calc() is not supported (see pool_style.rs), so resolve it to 0.
                    |_calc_value, _basis| 0.0,
                    |known_dimensions, available_space| {
                        measure_leaf_text(p_pool, item_index, known_dimensions, available_space)
                    },
                )
            } else if mode == LayoutMode::Flex {
                compute_flexbox_layout(tree, node_id, inputs)
            } else {
                // `None` = no extra block context (an Option, as in pool_tree.rs).
                compute_block_layout(tree, node_id, inputs, None)
            };
            if inputs.run_mode == RunMode::PerformLayout {
                // Places absolute/fixed boxes. Does nothing when there are none.
                compute_oof_layout(tree, node_id, &mut output);
            }
            // Last expression, no semicolon: the closure's result.
            output
        })
    }
}

// Block layout needs the style of containers and of their children.
impl LayoutBlockContainer for PoolLayoutTree {
    type BlockContainerStyle<'a>
        = PoolNodeStyleView<'a>
    where
        Self: 'a;
    type BlockItemStyle<'a>
        = PoolNodeStyleView<'a>
    where
        Self: 'a;

    fn get_block_container_style(&self, node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.style_view(node_id)
    }
    fn get_block_child_style(&self, child_node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.style_view(child_node_id)
    }
}

// Flexbox layout needs the same two getters.
impl LayoutFlexboxContainer for PoolLayoutTree {
    type FlexboxContainerStyle<'a>
        = PoolNodeStyleView<'a>
    where
        Self: 'a;
    type FlexboxItemStyle<'a>
        = PoolNodeStyleView<'a>
    where
        Self: 'a;

    fn get_flexbox_container_style(&self, node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.style_view(node_id)
    }
    fn get_flexbox_child_style(&self, child_node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.style_view(child_node_id)
    }
}

// Needed because `compute_root_layout` may position absolute/fixed boxes at the end.
impl LayoutContainingBlock for PoolLayoutTree {
    type OofItemStyle<'a>
        = PoolNodeStyleView<'a>
    where
        Self: 'a;

    fn get_oof_item_style(&self, node_id: NodeId) -> PoolNodeStyleView<'_> {
        self.style_view(node_id)
    }
    fn clear_hoisted_children(&mut self, node_id: NodeId) {
        let key: u64 = node_id.into();
        self.hoisted_children_per_node.remove(&key);
    }
    fn add_hoisted_children(&mut self, node_id: NodeId, hoisted: &[NodeId]) {
        let key: u64 = node_id.into();
        // `.or_default()` = "if missing, start with an empty Vec". `&[NodeId]` is a
        // borrowed run of NodeIds; `.extend_from_slice` appends copies of them.
        self.hoisted_children_per_node.entry(key).or_default().extend_from_slice(hoisted);
    }
}

// Taffy asks for a cache slot per node. We answer with the HashMap above.
impl CacheTree for PoolLayoutTree {
    fn cache_get(&mut self, node_id: NodeId, input: &LayoutInput) -> Option<LayoutOutput> {
        self.cache_for(node_id).get(input)
    }
    fn cache_store(&mut self, node_id: NodeId, input: &LayoutInput, layout_output: LayoutOutput) {
        self.cache_for(node_id).store(input, layout_output)
    }
    fn cache_clear(&mut self, node_id: NodeId) {
        // `.clear()` returns a value we do not need; the `;` throws it away, which makes
        // this function return nothing (`()`), as the trait demands.
        self.cache_for(node_id).clear();
    }
}

// -------------------------------------------------------------------------------------
// AFTER LAYOUT: relative -> absolute positions
// -------------------------------------------------------------------------------------

/// Taffy wrote x/y as offsets. Convert them to positions measured from the window's top-left
/// corner, like the rest of feav, by walking top-down.
///
/// What an offset is measured from:
///  * normal (in-flow) boxes:     their PARENT.
///  * absolute / fixed boxes:     their CONTAINING BLOCK (the nearest positioned ancestor, or
///                                the window for fixed). This is not always the parent.
/// Taffy records those absolute/fixed boxes per containing block; `containing_block_of`
/// is that record turned around: box -> its containing block.
unsafe fn make_positions_absolute(
    layout_tree: &PoolLayoutTree,
    containing_block_of: &HashMap<u64, u64>,
    node_id: NodeId,
    parent_absolute_x: f32,
    parent_absolute_y: f32,
) {
    let p_result = layout_tree.result_slot(node_id);
    let node_key: u64 = node_id.into();

    // Where this node's offset starts from.
    // `if let Some(x) = lookup { ... } else { ... }` = "if the lookup found something,
    // call it x and run the first block, otherwise run the second".
    let (origin_x, origin_y) = if let Some(containing_block_key) = containing_block_of.get(&node_key) {
        // The containing block is an ANCESTOR, so it was already converted: its x/y in the
        // pool are absolute now.
        let p_block_result = layout_tree.result_slot(NodeId::from(*containing_block_key));
        ((*p_block_result).x, (*p_block_result).y)
    } else {
        (parent_absolute_x, parent_absolute_y)
    };

    let absolute_x = origin_x + (*p_result).x;
    let absolute_y = origin_y + (*p_result).y;
    (*p_result).x = absolute_x;
    (*p_result).y = absolute_y;
    for child_id in layout_tree.child_ids(node_id) {
        make_positions_absolute(layout_tree, containing_block_of, child_id, absolute_x, absolute_y);
    }
}

// -------------------------------------------------------------------------------------
// THE DOOR C WALKS THROUGH
// -------------------------------------------------------------------------------------

/// C side:
/// `void feav_layout_compute(DOBJPool *pPool, ComputedElement *pResults, uint64_t resultCapacity, uint64_t rootGlobalId);`
///
/// * `pResults`       = the flat results array (C: `app->caches.results`), indexed by global id.
/// * `resultCapacity` = how many elements of it are committed:
///                      `app->caches.result_committed_size / sizeof(ComputedElement)`.
///
/// The viewport (window) size is read from the ROOT's result slot: `pResults[rootGlobalId].width`
/// and `.height`, where the parser stores it. The root is laid out at exactly that size. Every
/// node under the root gets `x`, `y` (window coordinates), `width`, `height` written into its
/// slot; the colour channels are left alone. Style arrays are never modified, so this can be
/// called again after a resize (update the root's width/height first) or a style change.
#[no_mangle]
pub unsafe extern "C" fn feav_layout_compute(
    p_pool: *mut DOBJPool,
    p_results: *mut ComputedElement,
    result_capacity: u64,
    root_global_id: u64,
) {
    compute_layout(p_pool, p_results, result_capacity, root_global_id);
}

/// The same work as `feav_layout_compute`, as a plain Rust function. (A panic inside an
/// `extern "C"` function cannot unwind into C: Rust aborts the process instead. Keeping the
/// real work in a plain function lets the tests check the failure cases.)
pub unsafe fn compute_layout(
    p_pool: *mut DOBJPool,
    p_results: *mut ComputedElement,
    result_capacity: u64,
    root_global_id: u64,
) {
    assert!(!p_pool.is_null(), "layout: the pool pointer is NULL");
    assert!(!p_results.is_null(), "layout: the results array is NULL (was it reserved and committed before parsing?)");
    // The struct needs its alignment; a misaligned array would make every access undefined.
    assert!(p_results as usize % align_of::<ComputedElement>() == 0, "layout: the results array is not aligned for ComputedElement");
    assert!(root_global_id < result_capacity, "layout: the root has no result slot");

    let root_node_id = NodeId::from(root_global_id);

    // The window size the parser stored in the root's result slot.
    let root_result = &*p_results.add(root_global_id as usize);
    let viewport = LayoutViewport { width: root_result.width, height: root_result.height, root_global_id };

    let mut layout_tree = PoolLayoutTree::new(p_pool, p_results, result_capacity as usize, viewport);
    // `AvailableSpace::Definite(n)` = "exactly n pixels of room" (other choices: min/max-content).
    let available_space =
        Size { width: AvailableSpace::Definite(viewport.width), height: AvailableSpace::Definite(viewport.height) };
    compute_root_layout(&mut layout_tree, root_node_id, available_space);

    // Turn Taffy's record "containing block -> boxes it positions" around into
    // "box -> its containing block".
    let mut containing_block_of: HashMap<u64, u64> = HashMap::new();
    for (containing_block_key, hoisted_boxes) in &layout_tree.hoisted_children_per_node {
        for hoisted_box in hoisted_boxes {
            containing_block_of.insert((*hoisted_box).into(), *containing_block_key);
        }
    }
    make_positions_absolute(&layout_tree, &containing_block_of, root_node_id, 0.0, 0.0);
}
