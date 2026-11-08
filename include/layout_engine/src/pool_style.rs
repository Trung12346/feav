// =====================================================================================
// pool_style.rs  --  Taffy reads a node's CSS style straight out of feav's pool
// =====================================================================================
//
// Terms (pool_tree.rs defines: global id, item index, pre-order):
//  * "style"    = the inputs Taffy needs for one node: size, margin, padding, position...
//  * "view"     = a tiny struct that copies no data. It remembers WHERE the data lives
//                 (pool pointer + slot number) and reads it when Taffy asks.
//  * "viewport" = the window area. Units like vw/vh/vmin/vmax are fractions of it.
//
// ADDING A CSS PROPERTY LATER = (1) add an array to the pool, (2) add one function to
// `impl CoreStyle` below that reads it. Nothing else in the layout code changes.

use core::marker::PhantomData;

use taffy::{
    BlockContainerStyle, BlockItemStyle, BoxGenerationMode, CoreStyle, Dimension, FlexboxContainerStyle,
    FlexboxItemStyle, LengthPercentage, LengthPercentageAuto, NodeId, OofItemStyle, Position, Rect, Size,
};

use crate::display::{layout_mode_of_node, LayoutMode};
use crate::pool_tree::PoolTree;
use crate::{
    global_id, DOBJPool, LayoutBox, Unit, UnitFlags, UNIT_OFFSET_BORDER_BOTTOM, UNIT_OFFSET_BORDER_LEFT,
    UNIT_OFFSET_BORDER_RIGHT, UNIT_OFFSET_BORDER_TOP, UNIT_OFFSET_HEIGHT, UNIT_OFFSET_INSET_BOTTOM,
    UNIT_OFFSET_INSET_LEFT, UNIT_OFFSET_INSET_RIGHT, UNIT_OFFSET_INSET_TOP, UNIT_OFFSET_MARGIN_BOTTOM,
    UNIT_OFFSET_MARGIN_LEFT, UNIT_OFFSET_MARGIN_RIGHT, UNIT_OFFSET_MARGIN_TOP, UNIT_OFFSET_PADDING_BOTTOM,
    UNIT_OFFSET_PADDING_LEFT, UNIT_OFFSET_PADDING_RIGHT, UNIT_OFFSET_PADDING_TOP, UNIT_OFFSET_WIDTH,
};

/// feav stores `Relative` values as the CSS percentage number (50 means 50%).
/// Taffy wants a fraction (0.5). This is the divisor between the two.
const RELATIVE_UNIT_DIVISOR: f32 = 100.0;
/// `vw`, `vh`, `vmin`, `vmax` values are also percentages of the viewport (10 = 10vw).
const VIEWPORT_UNIT_DIVISOR: f32 = 100.0;

/// The window the document is laid out in, and which node is the document root.
// `Clone, Copy` = this small struct can be duplicated freely (like a C struct passed by value).
#[derive(Clone, Copy, Debug)]
pub struct LayoutViewport {
    pub width: f32,
    pub height: f32,
    /// Global id of the root node. The root is always exactly viewport-sized.
    pub root_global_id: u64,
}

/// One node's style, as a view into the pool. Nothing is copied.
pub struct PoolNodeStyleView<'a> {
    p_pool: *const DOBJPool,
    item_index: usize,
    viewport: LayoutViewport,
    // Zero-byte lifetime marker, as in pool_tree.rs.
    _borrowed_from_tree: PhantomData<&'a PoolTree>,
}

// A second `impl PoolTree` block (Rust allows several, even in different files).
impl PoolTree {
    /// Make the style view for `node_id`.
    pub fn node_style_view(&self, node_id: NodeId, viewport: LayoutViewport) -> PoolNodeStyleView<'_> {
        let (pool_index, item_index) = PoolTree::split_node_id(node_id);
        let p_pool = unsafe { self.find_pool(pool_index) };
        PoolNodeStyleView { p_pool, item_index, viewport, _borrowed_from_tree: PhantomData }
    }
}

// -------------------------------------------------------------------------------------
// UNIT TRANSLATION: a feav (unit, number) pair  ->  something Taffy understands
// -------------------------------------------------------------------------------------

/// A size as feav means it, after resolving viewport units into pixels.
#[derive(Clone, Copy, Debug, PartialEq)]
enum ResolvedLength {
    Auto,
    Pixels(f32),
    /// A fraction of the parent's size: 0.5 = 50%.
    Fraction(f32),
}

/// `(unit, value)` -> `ResolvedLength`.
fn resolve_length(unit: Unit, value: f32, viewport: LayoutViewport) -> ResolvedLength {
    match unit {
        Unit::Auto => ResolvedLength::Auto,
        Unit::Pixel => ResolvedLength::Pixels(value),
        Unit::VWidth => ResolvedLength::Pixels(value / VIEWPORT_UNIT_DIVISOR * viewport.width),
        Unit::VHeight => ResolvedLength::Pixels(value / VIEWPORT_UNIT_DIVISOR * viewport.height),
        // `f32::min(a, b)` / `f32::max(a, b)` = the smaller / larger of two numbers.
        Unit::VMin => ResolvedLength::Pixels(value / VIEWPORT_UNIT_DIVISOR * viewport.width.min(viewport.height)),
        Unit::VMax => ResolvedLength::Pixels(value / VIEWPORT_UNIT_DIVISOR * viewport.width.max(viewport.height)),
        Unit::Relative => ResolvedLength::Fraction(value / RELATIVE_UNIT_DIVISOR),
        // calc() expressions are not stored anywhere in the pool yet, so there is nothing
        // to evaluate. Treated as Auto.
        Unit::FCalc => ResolvedLength::Auto,
    }
}

/// For width, height: Auto stays Auto.
fn to_dimension(length: ResolvedLength) -> Dimension {
    match length {
        ResolvedLength::Auto => Dimension::auto(),
        ResolvedLength::Pixels(pixels) => Dimension::length(pixels),
        ResolvedLength::Fraction(fraction) => Dimension::percent(fraction),
    }
}

/// For inset (top/right/bottom/left): Auto stays Auto, which is CSS's default.
fn to_length_percentage_auto_keeping_auto(length: ResolvedLength) -> LengthPercentageAuto {
    match length {
        ResolvedLength::Auto => LengthPercentageAuto::auto(),
        ResolvedLength::Pixels(pixels) => LengthPercentageAuto::length(pixels),
        ResolvedLength::Fraction(fraction) => LengthPercentageAuto::percent(fraction),
    }
}

/// For margin: an unset pool entry has the Auto unit, and an unset margin must be 0, not
/// CSS `margin: auto`. So Auto means 0 here. (Auto margins, used for centering, need a
/// separate "explicitly set" marker in the pool first.)
fn to_margin_length_treating_auto_as_zero(length: ResolvedLength) -> LengthPercentageAuto {
    match length {
        ResolvedLength::Auto => LengthPercentageAuto::length(0.0),
        ResolvedLength::Pixels(pixels) => LengthPercentageAuto::length(pixels),
        ResolvedLength::Fraction(fraction) => LengthPercentageAuto::percent(fraction),
    }
}

/// For padding and border: cannot be Auto in CSS, so Auto means 0.
fn to_length_percentage_treating_auto_as_zero(length: ResolvedLength) -> LengthPercentage {
    match length {
        ResolvedLength::Auto => LengthPercentage::length(0.0),
        ResolvedLength::Pixels(pixels) => LengthPercentage::length(pixels),
        ResolvedLength::Fraction(fraction) => LengthPercentage::percent(fraction),
    }
}

impl PoolNodeStyleView<'_> {
    /// Read the pool through the raw pointer. `unsafe` because the compiler cannot check
    /// the pointer; the promise is that C gave us a valid pool (see pool_tree.rs).
    #[inline]
    fn pool(&self) -> &DOBJPool {
        unsafe { &*self.p_pool }
    }

    fn global_id(&self) -> u64 {
        global_id(self.pool().pool_index, self.item_index)
    }

    fn is_root(&self) -> bool {
        self.global_id() == self.viewport.root_global_id
    }

    /// Unit of one property: 3 bits inside the node's packed 64-bit unit flags.
    fn unit_at(&self, unit_offset: u32) -> Unit {
        let flags: UnitFlags = self.pool().prty_flags[self.item_index];
        Unit::from_flags(flags, unit_offset)
    }

    /// Resolve one side of a four-sided property (margin, padding, inset).
    fn side_length(&self, unit_offset: u32, value: f32) -> ResolvedLength {
        resolve_length(self.unit_at(unit_offset), value, self.viewport)
    }
}

// -------------------------------------------------------------------------------------
// THE STYLE TRAITS
// -------------------------------------------------------------------------------------

// `CoreStyle` has many functions, and all of them have default answers except one
// associated type. We override the ones the pool can answer.
impl CoreStyle for PoolNodeStyleView<'_> {
    // Taffy lets styles contain named grid lines; those names need some string type.
    type CustomIdent = String;

    // `display: none`?
    fn box_generation_mode(&self) -> BoxGenerationMode {
        match layout_mode_of_node(self.pool(), self.item_index) {
            LayoutMode::Hidden => BoxGenerationMode::None,
            _ => BoxGenerationMode::Normal,
        }
    }

    // Taffy asks: "is this node `display: block`?". The default is `false` (Taffy's own
    // default display is flex). Block boxes must answer `true`, otherwise an Auto-width
    // node is not stretched to fill its parent.
    fn is_block(&self) -> bool {
        layout_mode_of_node(self.pool(), self.item_index) == LayoutMode::Block
    }

    // feav's `CSSattributePosition` numbers match Taffy's order: static, relative,
    // absolute, fixed, sticky.
    fn position(&self) -> Position {
        match self.pool().position[self.item_index] {
            1 => Position::Relative,
            2 => Position::Absolute,
            3 => Position::Fixed,
            4 => Position::Sticky,
            _ => Position::Static,
        }
    }

    // top / right / bottom / left offsets. Each side has its own unit.
    fn inset(&self) -> Rect<LengthPercentageAuto> {
        let inset: LayoutBox = self.pool().inset[self.item_index];
        Rect {
            top: to_length_percentage_auto_keeping_auto(self.side_length(UNIT_OFFSET_INSET_TOP, inset.top)),
            bottom: to_length_percentage_auto_keeping_auto(self.side_length(UNIT_OFFSET_INSET_BOTTOM, inset.bottom)),
            left: to_length_percentage_auto_keeping_auto(self.side_length(UNIT_OFFSET_INSET_LEFT, inset.left)),
            right: to_length_percentage_auto_keeping_auto(self.side_length(UNIT_OFFSET_INSET_RIGHT, inset.right)),
        }
    }

    // width, height. The ROOT is always exactly the viewport size, whatever its own
    // style says (that is what `html { width: 100vw; height: 100vh }` means for feav).
    fn size(&self) -> Size<Dimension> {
        if self.is_root() {
            return Size { width: Dimension::length(self.viewport.width), height: Dimension::length(self.viewport.height) };
        }
        let pool = self.pool();
        let width = self.side_length(UNIT_OFFSET_WIDTH, pool.width[self.item_index]);
        let height = self.side_length(UNIT_OFFSET_HEIGHT, pool.height[self.item_index]);
        Size { width: to_dimension(width), height: to_dimension(height) }
    }

    // Margin: every side has its own unit.
    fn margin(&self) -> Rect<LengthPercentageAuto> {
        let margin: LayoutBox = self.pool().margin[self.item_index];
        Rect {
            top: to_margin_length_treating_auto_as_zero(self.side_length(UNIT_OFFSET_MARGIN_TOP, margin.top)),
            bottom: to_margin_length_treating_auto_as_zero(self.side_length(UNIT_OFFSET_MARGIN_BOTTOM, margin.bottom)),
            left: to_margin_length_treating_auto_as_zero(self.side_length(UNIT_OFFSET_MARGIN_LEFT, margin.left)),
            right: to_margin_length_treating_auto_as_zero(self.side_length(UNIT_OFFSET_MARGIN_RIGHT, margin.right)),
        }
    }

    // Padding: every side has its own unit.
    fn padding(&self) -> Rect<LengthPercentage> {
        let padding: LayoutBox = self.pool().padding[self.item_index];
        Rect {
            top: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_PADDING_TOP, padding.top)),
            bottom: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_PADDING_BOTTOM, padding.bottom)),
            left: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_PADDING_LEFT, padding.left)),
            right: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_PADDING_RIGHT, padding.right)),
        }
    }

    // Border widths: every side has its own unit.
    fn border(&self) -> Rect<LengthPercentage> {
        let border: LayoutBox = self.pool().border[self.item_index];
        Rect {
            top: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_BORDER_TOP, border.top)),
            bottom: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_BORDER_BOTTOM, border.bottom)),
            left: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_BORDER_LEFT, border.left)),
            right: to_length_percentage_treating_auto_as_zero(self.side_length(UNIT_OFFSET_BORDER_RIGHT, border.right)),
        }
    }
    // min/max size, overflow, aspect ratio, box sizing: no pool data yet, Taffy's defaults apply.
}

// `impl Trait for Type {}` with EMPTY braces = "accept the default answer for every
// function of this trait". Block, flex and out-of-flow layout ask for container-only
// and item-only properties (flex direction, wrap, grow, shrink, text alignment, ...).
// The pool stores none of them yet, so the CSS initial values (the defaults) are correct.
// To support one later, move it out of the empty braces and read it from the pool.
impl BlockContainerStyle for PoolNodeStyleView<'_> {}
impl BlockItemStyle for PoolNodeStyleView<'_> {}
impl OofItemStyle for PoolNodeStyleView<'_> {}
impl FlexboxContainerStyle for PoolNodeStyleView<'_> {}
impl FlexboxItemStyle for PoolNodeStyleView<'_> {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn units_translate_to_taffy_lengths() {
        let viewport = LayoutViewport { width: 800.0, height: 600.0, root_global_id: 0 };
        assert_eq!(resolve_length(Unit::Auto, 7.0, viewport), ResolvedLength::Auto);
        assert_eq!(resolve_length(Unit::Pixel, 100.0, viewport), ResolvedLength::Pixels(100.0));
        assert_eq!(resolve_length(Unit::Relative, 50.0, viewport), ResolvedLength::Fraction(0.5));
        assert_eq!(resolve_length(Unit::VWidth, 10.0, viewport), ResolvedLength::Pixels(80.0));
        assert_eq!(resolve_length(Unit::VHeight, 10.0, viewport), ResolvedLength::Pixels(60.0));
        assert_eq!(resolve_length(Unit::VMin, 10.0, viewport), ResolvedLength::Pixels(60.0));
        assert_eq!(resolve_length(Unit::VMax, 10.0, viewport), ResolvedLength::Pixels(80.0));
        assert_eq!(to_dimension(ResolvedLength::Pixels(100.0)), Dimension::length(100.0));
        assert_eq!(to_margin_length_treating_auto_as_zero(ResolvedLength::Auto), LengthPercentageAuto::length(0.0));
        assert_eq!(to_length_percentage_auto_keeping_auto(ResolvedLength::Auto), LengthPercentageAuto::auto());
    }
}
