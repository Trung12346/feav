// =====================================================================================
// pool_tree.rs  --  Step 1: let Taffy SEE feav's memory pool. No layout yet.
// =====================================================================================
//
// HOW TO READ THIS FILE IF YOU KNOW NO RUST
// -----------------------------------------
// Every Rust construct gets a comment the FIRST time it appears. Later uses are
// explained again only briefly. Comments start with `//` and run to the end of the line.
// A comment starting with `///` (three slashes) is a "doc comment": the same thing, but
// Rust tools attach it to the item right below it. For you, it is just a normal comment.
//
// Vocabulary specific to feav (used throughout):
//  * "global id"  = pool_index * POOL_ELEMENT_COUNT + item_index.
//                   One number that names one node across ALL pools.
//  * "item index" = the slot number (0..1023) of a node inside ONE pool.
//  * "pre-order"  = the order your parser stores nodes: a parent, then its first child
//                   directly after it, then that child's descendants, then the next
//                   sibling, and so on. Consequence: the first child of a node is
//                   always the slot right after the node.
//
// Vocabulary specific to Taffy:
//  * "NodeId"     = Taffy's name for "one number that identifies one node".
//                   We feed it your global id.
//  * "trait"      = see the first `impl ... for ...` below.

// -------------------------------------------------------------------------------------
// IMPORTS
// -------------------------------------------------------------------------------------

// `use` brings a name from somewhere else into this file, so we can write the short name
// instead of the full path. It is like `#include`, but for single names, not whole files.
// `::` separates the pieces of a path (core -> marker -> PhantomData).
// `PhantomData` is a zero-byte marker. It is explained where it is used (PoolChildIterator).
use core::marker::PhantomData;

// Curly braces `{ }` in a `use` list several names from the same place.
// `NodeId` and `TraversePartialTree` both come from the external library `taffy`.
// (`TraversePartialTree` is a trait, explained at its first use below.)
use taffy::{NodeId, TraversePartialTree};

// `crate` means "this Rust library itself" (feav_layout). These three things are defined
// in lib.rs: `global_id` is a function, `DOBJPool` is the Rust mirror of your C struct,
// and `POOL_ELEMENT_COUNT` is the slot count per pool (1024).
use crate::{global_id, DOBJPool, POOL_ELEMENT_COUNT};

// -------------------------------------------------------------------------------------
// THE TREE HANDLE
// -------------------------------------------------------------------------------------

/// Taffy never owns the pool. This struct only remembers where C's pool lives.
///
// `pub` = "public": visible to other files. Without it, things are private to this file.
// `struct` = a named bundle of fields. Same idea as a C struct.
pub struct PoolTree {
    /// Any pool of the linked list (first, last, whichever C has at hand).
    ///
    // `p_any_pool` is the field name. After the colon comes its type.
    // `*const DOBJPool` is a "raw pointer": exactly like C's `const DOBJPool *`.
    // The compiler does NOT check raw pointers, so reading through one needs `unsafe`
    // (explained at the first `unsafe`). `const` means: we only read through it.
    // The trailing comma after the last field is allowed and customary in Rust.
    pub p_any_pool: *const DOBJPool,
}

// `impl PoolTree { ... }` = "here are the functions that belong to PoolTree".
// Like writing functions that take a `PoolTree *self` in C, but grouped under the type.
impl PoolTree {
    /// Split a Taffy `NodeId` back into (pool index, item index).
    ///
    // `#[inline]` is an "attribute" (square brackets after `#`). Attributes are notes
    // to the compiler. This one says: "paste this function's body into its callers",
    // like `static inline` in C. It does not change behaviour.
    #[inline]
    // `fn` defines a function. Parameters are written `name: Type`.
    // `-> (u16, usize)` is the return type. `(A, B)` is a TUPLE: an unnamed pair of
    // values. `u16` = unsigned 16-bit integer (uint16_t). `usize` = unsigned integer
    // as wide as a pointer (size_t).
    // This function has no `self` parameter, so it is called as `Self::split_node_id(x)`,
    // not as `something.split_node_id(x)`. `Self` means "the type we are inside", here PoolTree.
    // `pub(crate)` = visible to every file of this library (this crate), but not to outside
    // code. It sits between private (this file only) and `pub` (everyone). Step 2's
    // new file needs these two helpers, so they are widened from private to this level.
    pub(crate) fn split_node_id(node_id: NodeId) -> (u16, usize) {
        // `let` creates a variable. Variables cannot be changed after creation unless
        // declared `let mut` (see find_pool). `: u64` states the variable's type.
        // `.into()` converts a value into another type; the annotation `u64` tells
        // it which type to convert into (NodeId -> u64).
        let global: u64 = node_id.into();
        // `as` is a numeric cast, same as C's `(uint64_t)x`.
        // `/` and `%` are integer divide and remainder, as in C.
        let pool_index = (global / POOL_ELEMENT_COUNT as u64) as u16;
        let item_index = (global % POOL_ELEMENT_COUNT as u64) as usize;
        // A line WITHOUT a trailing semicolon at the end of a function is its return
        // value. This is the same as `return (pool_index, item_index);` in C.
        (pool_index, item_index)
    }

    /// Walk the doubly linked pool list until the pool with `target_pool_index` is found.
    /// Linear walk on purpose: simplest thing that works. Cache it later.
    #[inline]
    // `unsafe fn` = a function the compiler cannot prove is safe to call. The author
    // (us) promises it is fine if the caller follows the rules. Here the rule is:
    // `p_any_pool` must point to a valid pool, and the pool list links must be valid.
    // Anyone calling it must write `unsafe { ... }` around the call.
    //
    // `&self` = "borrow this PoolTree for reading". Like `const PoolTree *self` in C.
    // `&` means "reference": a pointer the compiler checks for you. Using `&` on a
    // value never copies it and never gives up ownership.
    // The `-> *const DOBJPool` says this returns a raw read-only pointer.
    pub(crate) unsafe fn find_pool(&self, target_pool_index: u16) -> *const DOBJPool {
        // `mut` = this variable may be reassigned later.
        let mut p_pool = self.p_any_pool;
        // `while condition { ... }` loops as long as the condition is true, as in C.
        // `(*p_pool)` = "the thing the pointer points at" (dereference, same `*` as C).
        // The parentheses are needed because `.` binds tighter than `*` in Rust,
        // so `*p_pool.pool_index` would mean something else.
        // `(*p_pool).pool_index` is C's `p_pool->pool_index`.
        while (*p_pool).pool_index > target_pool_index {
            // `p_prev` is a `*mut DOBJPool` (writable raw pointer). Rust quietly
            // lets a `*mut` be stored where a `*const` is expected.
            p_pool = (*p_pool).p_prev;
        }
        while (*p_pool).pool_index < target_pool_index {
            p_pool = (*p_pool).p_next;
        }
        p_pool
    }
}

// -------------------------------------------------------------------------------------
// THE CHILD ITERATOR
// -------------------------------------------------------------------------------------

/// Iterator over the children of ONE node. Taffy calls this to ask "who are your children?".
///
// `<'a>` after the name declares a LIFETIME named `a`. (The apostrophe is part of the
// syntax.) A lifetime is a label for "how long a borrowed thing stays valid". Here it
// says: this iterator may not outlive the PoolTree it was made from. The compiler
// uses it to stop bugs like using a pointer after the data was freed. You never have
// to do anything at runtime for it; it exists only while compiling.
pub struct PoolChildIterator<'a> {
    // The pool that holds the child we will hand out next.
    p_current_pool: *const DOBJPool,
    // The slot of that child inside its pool.
    current_item_index: usize,
    // How many children we still have to hand out. Counting down to 0 tells us to stop.
    remaining_child_count: usize,
    // `PhantomData<&'a PoolTree>` is a field that takes ZERO bytes. It exists only to
    // tell the compiler "pretend this struct holds a borrow of a PoolTree for
    // lifetime 'a". Raw pointers carry no lifetime information, so without this the
    // `'a` above would have nothing to attach to and the compiler would reject it.
    // A name starting with `_` tells the compiler "I know this field is never read".
    _borrowed_from_tree: PhantomData<&'a PoolTree>,
}

// A TRAIT is a list of functions that a type promises to provide, like a table of
// function pointers or an interface. `Iterator` is a trait from Rust's standard
// library. Anything that implements it can be used in a `for ... in` loop.
// `impl Trait for Type { ... }` = "here is how Type provides the functions Trait wants".
// `'_` = "some lifetime, I do not care which", needed because the struct has an `'a`.
impl Iterator for PoolChildIterator<'_> {
    // `type Item = ...;` is part of the Iterator trait: it states what one loop step
    // produces. For us, each step gives one `NodeId`.
    type Item = NodeId;

    // The single function Iterator requires. Each call produces the next child.
    // `&mut self` = borrow this iterator for reading AND writing (we change counters).
    // `Option<NodeId>` is a built-in type with exactly two possible values:
    //   `Some(x)` = "here is a value, x"
    //   `None`    = "no more values"
    // It replaces C's habit of returning NULL or -1 to mean "nothing".
    fn next(&mut self) -> Option<NodeId> {
        // `if condition { ... }`: same as C but no parentheses needed around the condition.
        // `==` is comparison. `return` leaves the function early, as in C.
        if self.remaining_child_count == 0 {
            return None;
        }
        // `unsafe { ... }` = a block where we do things the compiler cannot verify.
        // `*self.p_current_pool` dereferences the raw pointer; `&` in front then makes
        // a normal checked reference (`&DOBJPool`) so the lines below can read fields
        // with a plain `.` instead of `(*p)->`. We are promising the pointer is valid.
        let pool = unsafe { &*self.p_current_pool };
        // `NodeId::from(x)` = "make a NodeId out of x". `global_id(a, b)` is your
        // function from lib.rs that computes pool_index * 1024 + item_index.
        let this_child = NodeId::from(global_id(pool.pool_index, self.current_item_index));
        // `-=` subtracts in place, as in C.
        self.remaining_child_count -= 1;

        // Only follow the sibling link if another child is still expected;
        // the LAST child's next-sibling slot holds a null pool pointer.
        if self.remaining_child_count > 0 {
            // `[ ... ]` indexes an array, as in C. Unlike C, Rust checks the index and
            // stops the program with an error message if it is out of range.
            let next_pool = pool.pool_next_sib[self.current_item_index];
            // `as usize` converts the stored 16-bit index into a `usize` so it can be
            // used as an index again.
            let next_item_index = pool.idx_next_sib[self.current_item_index] as usize;
            self.p_current_pool = next_pool;
            self.current_item_index = next_item_index;
        }
        // `Some(this_child)` wraps the value into the "there is a value" case of Option.
        // No semicolon: this is the function's return value.
        Some(this_child)
    }
}

// -------------------------------------------------------------------------------------
// TEACHING TAFFY HOW TO WALK OUR TREE
// -------------------------------------------------------------------------------------

// `TraversePartialTree` is a trait defined by Taffy. Implementing it for PoolTree is the
// whole trick of this step: it tells Taffy "to find children, call these three functions".
// Taffy's layout code is written against this trait, not against any one tree type,
// so it will work on top of our pool.
impl TraversePartialTree for PoolTree {
    // Taffy's trait asks for an associated type named `ChildIter`: "what type do you
    // use to list children?". Our answer is PoolChildIterator.
    // `<'a>` is a lifetime again. `where Self: 'a` means: "this only holds while the
    // PoolTree itself is still alive for the lifetime 'a". Taffy's trait demands this
    // exact wording, so we copy it.
    type ChildIter<'a>
        = PoolChildIterator<'a>
    where
        Self: 'a;

    // Taffy asks: "give me the children of this node".
    // `Self::ChildIter<'_>` is the type declared just above (PoolChildIterator).
    fn child_ids(&self, parent_node_id: NodeId) -> Self::ChildIter<'_> {
        // `let (a, b) = ...;` takes a tuple apart into two variables at once.
        let (parent_pool_index, parent_item_index) = Self::split_node_id(parent_node_id);
        // Calling an `unsafe fn` is only allowed inside an `unsafe { }` block.
        let p_parent_pool = unsafe { self.find_pool(parent_pool_index) };
        // `(*p_parent_pool).child_count[...]` reads your C field `child_count[i]`.
        // `as usize` converts it to the counting type Rust uses for sizes.
        let child_count = unsafe { (*p_parent_pool).child_count[parent_item_index] } as usize;

        // First child = the slot right after the parent (pre-order), possibly in the next pool.
        // In Rust `if ... { a } else { b }` is an EXPRESSION: it produces a value, so it
        // can be assigned directly to variables. Both branches must give the same type,
        // here a tuple of (pointer, number).
        let (p_first_child_pool, first_child_item_index) =
            if parent_item_index + 1 == POOL_ELEMENT_COUNT {
                // Parent is the last slot: the child is slot 0 of the next pool.
                // `as *const DOBJPool` casts the writable pointer into a read-only one.
                (unsafe { (*p_parent_pool).p_next as *const DOBJPool }, 0)
            } else {
                (p_parent_pool, parent_item_index + 1)
            };

        // Building a struct: `TypeName { field: value, ... }`. This last expression
        // (no semicolon) is what the function returns.
        PoolChildIterator {
            p_current_pool: p_first_child_pool,
            current_item_index: first_child_item_index,
            remaining_child_count: child_count,
            // Creates the zero-byte marker value.
            _borrowed_from_tree: PhantomData,
        }
    }

    // Taffy asks: "how many children does this node have?".
    fn child_count(&self, parent_node_id: NodeId) -> usize {
        let (parent_pool_index, parent_item_index) = Self::split_node_id(parent_node_id);
        let p_parent_pool = unsafe { self.find_pool(parent_pool_index) };
        unsafe { (*p_parent_pool).child_count[parent_item_index] as usize }
    }

    // Taffy asks: "give me the n-th child (counting from 0) of this node".
    fn get_child_id(&self, parent_node_id: NodeId, child_index: usize) -> NodeId {
        // O(child_index) for now (walks siblings). Fine for step 1.
        // `self.child_ids(...)` is our function above; `.nth(n)` is a method every
        // iterator has: "skip n items and give me the next one" (as an Option).
        // `.expect("text")` turns `Some(x)` into `x`, and stops the program with the
        // text as the error message if the Option was `None`.
        self.child_ids(parent_node_id).nth(child_index).expect("child_index out of range")
    }
}

// -------------------------------------------------------------------------------------
// TESTS (not part of your program; only compiled when you run `cargo test`)
// -------------------------------------------------------------------------------------

// `#[cfg(test)]` = "only compile the next item when building tests".
// `mod tests { ... }` = a named group of code inside this file (a "module").
#[cfg(test)]
mod tests {
    // `super` = the module that contains this one (the file's top level).
    // `*` = "everything public or visible there". So the tests can use PoolTree etc.
    use super::*;
    // Memory-allocation functions from Rust's standard library (`std`):
    //   alloc_zeroed = calloc, dealloc = free, Layout = "size and alignment of a type".
    use std::alloc::{alloc_zeroed, dealloc, Layout};

    // Build an empty pool whose `pool_index` is set. Returns a WRITABLE raw pointer
    // (`*mut`), because the tests need to fill it in afterwards.
    unsafe fn new_pool(pool_index: u16) -> *mut DOBJPool {
        // `Layout::new::<DOBJPool>()` : the `::<Type>` part (called "turbofish") names
        // the type explicitly: "the memory layout of a DOBJPool".
        // `alloc_zeroed` returns a byte pointer (`*mut u8`), so we cast it to a
        // pointer to the struct, same as `(DOBJPool *)calloc(1, sizeof(DOBJPool))`.
        let p = alloc_zeroed(Layout::new::<DOBJPool>()) as *mut DOBJPool;
        (*p).pool_index = pool_index;
        p
    }

    /// Test helper: count every node below `node_id` using only Taffy's trait.
    fn count_descendants<T: TraversePartialTree>(tree: &T, node_id: NodeId) -> u64 {
        let mut descendant_count = 0u64;
        for child_id in tree.child_ids(node_id) {
            descendant_count += 1 + count_descendants(tree, child_id);
        }
        descendant_count
    }

    // `#[test]` marks a function as a test. `cargo test` runs each one and reports
    // pass or fail. A test fails if it stops with an error.
    #[test]
    fn walks_nested_tree_in_one_pool() {
        unsafe {
            let p = new_pool(0);
            // `&mut *p` = turn the raw pointer into a checked, writable reference.
            let pool = &mut *p;
            // 0 body -> children 1, 2, 4 ; 2 has one child 3
            pool.child_count[0] = 3;
            pool.child_count[2] = 1;
            // `[(1usize, 2u16), (2, 4)]` is an ARRAY holding two tuples.
            // `for (from, to) in ...` takes each tuple apart: first (1, 2), then (2, 4).
            // Meaning: slot 1's next sibling is slot 2; slot 2's next sibling is slot 4.
            for (from, to) in [(1usize, 2u16), (2, 4)] {
                pool.pool_next_sib[from] = p;
                pool.idx_next_sib[from] = to;
            }
            let tree = PoolTree { p_any_pool: p };
            // `assert_eq!(a, b)` is a macro: if a and b differ, the test fails.
            // Here: node 0 has 4 descendants in total (1, 2, 3, 4).
            assert_eq!(count_descendants(&tree, NodeId::from(0u64)), 4);
            // `Vec<u64>` = "Vec" is a growable array (like a malloc'd array plus its
            // length), here holding u64 values.
            // `.map(Into::into)` converts each NodeId into u64.
            // `.collect()` gathers all produced items into the Vec named by the
            // annotation. Without `: Vec<u64>` the compiler would not know what to gather into.
            let kids: Vec<u64> = tree.child_ids(NodeId::from(0u64)).map(Into::into).collect();
            // `vec![1, 2, 4]` is a macro that builds a Vec with those values.
            assert_eq!(kids, vec![1, 2, 4]);
            assert_eq!(tree.child_count(NodeId::from(2u64)), 1);
            // Free the memory: `p as *mut u8` casts back to bytes, as free() would want.
            dealloc(p as *mut u8, Layout::new::<DOBJPool>());
        }
    }

    #[test]
    fn first_child_can_live_in_the_next_pool() {
        unsafe {
            let p0 = new_pool(0);
            let p1 = new_pool(1);
            // Link the two pools to each other, like your C doubly linked list.
            (*p0).p_next = p1;
            (*p1).p_prev = p0;
            (*p0).child_count[1023] = 1; // parent = last slot of pool 0
            let tree = PoolTree { p_any_pool: p1 }; // start from the OTHER pool on purpose
            let kids: Vec<u64> = tree.child_ids(NodeId::from(1023u64)).map(Into::into).collect();
            assert_eq!(kids, vec![1024]); // pool 1, slot 0
            dealloc(p0 as *mut u8, Layout::new::<DOBJPool>());
            dealloc(p1 as *mut u8, Layout::new::<DOBJPool>());
        }
    }
}
