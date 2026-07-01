//! `std::thread`, `Arc<Mutex<_>>`, and mpsc channels.
#![allow(dead_code, unused_variables)]

/// Sum `xs` using exactly `nthreads` OS threads (or fewer if there are
/// fewer elements than threads; an empty vec returns 0 without spawning).
///
/// Approach: split `xs` into `nthreads` contiguous chunks, `thread::spawn`
/// one summing closure per chunk (each closure takes ownership of its
/// chunk — `move`), then `join` and add the partial sums.
///
/// Panics in a worker should propagate (just `unwrap()` the join result).
pub fn parallel_sum(xs: Vec<i64>, nthreads: usize) -> i64 {
    todo!("implement parallel_sum")
}

/// Spawn `nthreads` threads that each increment a shared counter
/// `increments` times, then return the final count — which must be
/// **exactly** `nthreads * increments` (that's the point of the `Mutex`).
///
/// Use `Arc<Mutex<usize>>`: `Arc` shares ownership across threads,
/// `Mutex` makes the increment atomic.
pub fn shared_counter(nthreads: usize, increments: usize) -> usize {
    todo!("implement shared_counter")
}

/// A two-stage pipeline over channels:
///
/// ```text
/// main --(tx1)--> stage 1: double each --(tx2)--> stage 2: sum --> result
/// ```
///
/// Create two `std::sync::mpsc::channel`s and two threads. Stage 1 receives
/// `i64`s, sends each `* 2` downstream. Stage 2 receives the doubled values
/// and sums them. Main sends every element of `xs` into the first channel,
/// then **drops the sender** (hang alert: if a sender stays alive, the
/// receiver loop never ends). Returns the sum from stage 2's join.
///
/// - `pipeline_double_then_sum(vec![1, 2, 3])` → `12`
pub fn pipeline_double_then_sum(xs: Vec<i64>) -> i64 {
    todo!("implement pipeline_double_then_sum")
}
