//! Ownership, borrowing, and lifetimes.
//!
//! Every function body is `todo!()` — the crate compiles, but each test fails
//! with a panic until you implement it. Delete the `todo!()` and go.
#![allow(dead_code, unused_variables)]

use std::collections::HashMap;

/// Return `s` uppercased with a single `'!'` appended.
///
/// - `shout("hello")` → `"HELLO!"`
/// - `shout("")` → `"!"`
///
/// Takes a borrowed `&str` and returns an owned `String` — the classic
/// "borrow in, own out" signature.
pub fn shout(s: &str) -> String {
    todo!("implement shout")
}

/// Return the longer of the two string slices. On a tie, return `a`.
///
/// The explicit lifetime `'a` says: the returned reference lives no longer
/// than the *shorter-lived* of the two inputs.
pub fn longest<'a>(a: &'a str, b: &'a str) -> &'a str {
    todo!("implement longest")
}

/// Count occurrences of every `char` (Unicode scalar value) in `s`,
/// including whitespace and punctuation.
///
/// - `char_freq("aab")` → `{'a': 2, 'b': 1}`
/// - `char_freq("")` → empty map
pub fn char_freq(s: &str) -> HashMap<char, usize> {
    todo!("implement char_freq")
}

/// Consume `v` (note: takes it by value, the caller can't use it afterwards)
/// and return only the even elements, preserving their original order.
///
/// `0` is even; negative even numbers count too.
///
/// - `take_evens(vec![1, 2, 3, 4])` → `[2, 4]`
pub fn take_evens(v: Vec<i32>) -> Vec<i32> {
    todo!("implement take_evens")
}

/// Sum a borrowed slice of `i32`s, widening to `i64` so the sum cannot
/// overflow (e.g. `sum_refs(&[i32::MAX, i32::MAX])` must be exact).
pub fn sum_refs(xs: &[i32]) -> i64 {
    todo!("implement sum_refs")
}
