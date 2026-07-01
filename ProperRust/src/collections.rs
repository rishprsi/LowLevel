//! Vec, HashMap, iterators, and sorting.
#![allow(dead_code, unused_variables)]

use std::collections::HashMap;

/// Remove adjacent duplicate elements from `v` in place, keeping the first
/// occurrence of each run. Since the input is assumed sorted, this removes
/// *all* duplicates.
///
/// - `[1, 1, 2, 3, 3, 3]` → `[1, 2, 3]`
/// - `[]` → `[]`
pub fn dedup_sorted(v: &mut Vec<i32>) {
    todo!("implement dedup_sorted")
}

/// Count words in `text`, split on any whitespace, lowercased before
/// counting. Punctuation is NOT stripped: `"hi,"` and `"hi"` are different
/// words.
///
/// - `word_count("The the THE")` → `{"the": 3}`
pub fn word_count(text: &str) -> HashMap<String, usize> {
    todo!("implement word_count")
}

/// Return up to `k` `(word, count)` pairs from `freq`, sorted by count
/// descending, ties broken by word ascending (lexicographic).
///
/// If `freq` has fewer than `k` entries, return them all. `k == 0` returns
/// an empty vec.
pub fn top_k(freq: &HashMap<String, usize>, k: usize) -> Vec<(String, usize)> {
    todo!("implement top_k")
}

/// Rotate `v` left by `k` positions in place. `k` may exceed `v.len()`
/// (rotate by `k % len`). An empty vec is a no-op (careful: don't divide
/// by zero).
///
/// - `[1, 2, 3, 4, 5]` rotated left by 2 → `[3, 4, 5, 1, 2]`
pub fn rotate_left<T>(v: &mut Vec<T>, k: usize) {
    todo!("implement rotate_left")
}
