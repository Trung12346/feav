// =====================================================================================
// text_measure.rs  --  how Taffy learns how big a piece of text is
// =====================================================================================
//
// Taffy does not know fonts. For a node with no element children (a "leaf") it asks:
// "how big are you?". We forward that question to a C function that feav registers once.
// C knows where the node's text lives and what the font metrics are; Rust does not.
//
// The contract, from Taffy's side:
//  * `width_mode`      = what kind of width limit applies (see LayoutAvailableSpaceMode).
//  * `available_width` = the limit in pixels (only meaningful for the Definite mode).
//  * C answers with the width and height the text needs under that limit.

use core::ffi::c_void;
use std::sync::Mutex;

use taffy::{AvailableSpace, Size};

use crate::DOBJPool;

/// How wide may the text be? (C: `LayoutAvailableSpaceMode`)
// `#[repr(i32)]` = stored as a 32-bit integer, like a C enum.
#[repr(i32)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LayoutAvailableSpaceMode {
    /// Wrap at exactly `available_width` pixels.
    Definite = 0,
    /// Wrap at every possible break: the narrowest the text can be.
    MinContent = 1,
    /// Never wrap: the widest the text wants to be.
    MaxContent = 2,
}

/// The answer written by the C callback. (C: `LayoutTextMeasureResult`)
// `#[repr(C)]` = lay the fields out in memory exactly like a C struct.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default)]
pub struct LayoutTextMeasureResult {
    pub width: f32,
    pub height: f32,
}

/// The shape of the C function feav registers. (C: `PFN_LayoutMeasureText`)
/// Returns 0 if the node has no text (Rust then treats it as 0 x 0), nonzero if it has text.
/// (A plain 32-bit integer, not a `bool`: feav's own C `bool` is a 4-byte enum.)
// `unsafe extern "C" fn(...) -> bool` is the TYPE of a C function pointer.
pub type TextMeasureCallback = unsafe extern "C" fn(
    p_user_data: *mut c_void,
    p_pool: *const DOBJPool,
    item_index: u32,
    width_mode: i32,
    available_width: f32,
    p_result: *mut LayoutTextMeasureResult,
) -> i32;

// The registered callback plus its user data pointer (stored as a plain number, because a
// raw pointer cannot be shared between threads in Rust, a number can).
// `static` = one global variable for the whole program. `Mutex<T>` = a lock around a value,
// so only one piece of code touches it at a time.
static REGISTERED_TEXT_MEASURE: Mutex<Option<(TextMeasureCallback, usize)>> = Mutex::new(None);

/// C side: `void feav_layout_set_text_measure(PFN_LayoutMeasureText pfnMeasureText, void *pUserData);`
/// Pass NULL to unregister (every leaf then measures 0 x 0).
// `Option<unsafe extern "C" fn ...>` is how Rust receives a C function pointer that may
// be NULL: `None` = NULL, `Some(f)` = a real function.
#[no_mangle]
pub unsafe extern "C" fn feav_layout_set_text_measure(callback: Option<TextMeasureCallback>, p_user_data: *mut c_void) {
    // `.lock()` takes the lock. `if let Ok(mut guard) = ...` runs the block only if that
    // worked (it can fail if another thread crashed while holding the lock).
    if let Ok(mut guard) = REGISTERED_TEXT_MEASURE.lock() {
        // `*guard = ...` replaces the stored value. `callback.map(|f| (f, ptr))` turns
        // `Some(f)` into `Some((f, ptr))` and keeps `None` as `None`.
        *guard = callback.map(|function| (function, p_user_data as usize));
    }
}

/// Ask C how big a leaf's text is. Used as Taffy's "measure function".
pub(crate) fn measure_leaf_text(
    p_pool: *const DOBJPool,
    item_index: usize,
    known_dimensions: Size<Option<f32>>,
    available_space: Size<AvailableSpace>,
) -> Size<f32> {
    // Decide what to tell C about the width limit. A width Taffy already knows wins.
    let (width_mode, available_width) = match (known_dimensions.width, available_space.width) {
        (Some(known_width), _) => (LayoutAvailableSpaceMode::Definite, known_width),
        (None, AvailableSpace::Definite(width)) => (LayoutAvailableSpaceMode::Definite, width),
        (None, AvailableSpace::MinContent) => (LayoutAvailableSpaceMode::MinContent, 0.0),
        (None, AvailableSpace::MaxContent) => (LayoutAvailableSpaceMode::MaxContent, 0.0),
    };

    let mut measured = LayoutTextMeasureResult::default();
    // Copy the registration out so the lock is released before calling into C.
    let registration = REGISTERED_TEXT_MEASURE.lock().ok().and_then(|guard| *guard);
    if let Some((callback, user_data)) = registration {
        let has_text = 0 != unsafe {
            callback(user_data as *mut c_void, p_pool, item_index as u32, width_mode as i32, available_width, &mut measured)
        };
        if !has_text {
            measured = LayoutTextMeasureResult::default();
        }
    }
    // Known dimensions override what C measured.
    Size {
        width: known_dimensions.width.unwrap_or(measured.width),
        height: known_dimensions.height.unwrap_or(measured.height),
    }
}
