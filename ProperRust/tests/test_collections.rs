use proper_rust::collections::*;
use std::collections::HashMap;

fn freq_of(pairs: &[(&str, usize)]) -> HashMap<String, usize> {
    pairs.iter().map(|(w, c)| (w.to_string(), *c)).collect()
}

#[test]
fn dedup_sorted_basic() {
    let mut v = vec![1, 1, 2, 3, 3, 3, 4];
    dedup_sorted(&mut v);
    assert_eq!(v, vec![1, 2, 3, 4]);
}

#[test]
fn dedup_sorted_no_dups() {
    let mut v = vec![1, 2, 3];
    dedup_sorted(&mut v);
    assert_eq!(v, vec![1, 2, 3]);
}

#[test]
fn dedup_sorted_all_same() {
    let mut v = vec![7, 7, 7, 7];
    dedup_sorted(&mut v);
    assert_eq!(v, vec![7]);
}

#[test]
fn dedup_sorted_empty_and_single() {
    let mut v: Vec<i32> = vec![];
    dedup_sorted(&mut v);
    assert!(v.is_empty());

    let mut v = vec![42];
    dedup_sorted(&mut v);
    assert_eq!(v, vec![42]);
}

#[test]
fn word_count_basic() {
    let wc = word_count("the quick the lazy the");
    assert_eq!(wc.get("the"), Some(&3));
    assert_eq!(wc.get("quick"), Some(&1));
    assert_eq!(wc.get("lazy"), Some(&1));
    assert_eq!(wc.len(), 3);
}

#[test]
fn word_count_lowercases() {
    let wc = word_count("The THE the tHe");
    assert_eq!(wc.get("the"), Some(&4));
    assert_eq!(wc.len(), 1);
}

#[test]
fn word_count_any_whitespace() {
    let wc = word_count("a\tb\nc  a");
    assert_eq!(wc.get("a"), Some(&2));
    assert_eq!(wc.get("b"), Some(&1));
    assert_eq!(wc.get("c"), Some(&1));
}

#[test]
fn word_count_punctuation_not_stripped() {
    let wc = word_count("hi, hi");
    assert_eq!(wc.get("hi,"), Some(&1));
    assert_eq!(wc.get("hi"), Some(&1));
}

#[test]
fn word_count_empty() {
    assert!(word_count("").is_empty());
    assert!(word_count("   \n\t ").is_empty());
}

#[test]
fn top_k_sorted_by_count_desc() {
    let freq = freq_of(&[("apple", 3), ("banana", 5), ("cherry", 1)]);
    assert_eq!(
        top_k(&freq, 2),
        vec![("banana".to_string(), 5), ("apple".to_string(), 3)]
    );
}

#[test]
fn top_k_ties_broken_by_word_asc() {
    let freq = freq_of(&[("pear", 2), ("apple", 2), ("mango", 2)]);
    assert_eq!(
        top_k(&freq, 3),
        vec![
            ("apple".to_string(), 2),
            ("mango".to_string(), 2),
            ("pear".to_string(), 2)
        ]
    );
}

#[test]
fn top_k_k_larger_than_map() {
    let freq = freq_of(&[("solo", 1)]);
    assert_eq!(top_k(&freq, 10), vec![("solo".to_string(), 1)]);
}

#[test]
fn top_k_zero_and_empty() {
    let freq = freq_of(&[("a", 1)]);
    assert!(top_k(&freq, 0).is_empty());
    assert!(top_k(&HashMap::new(), 5).is_empty());
}

#[test]
fn rotate_left_basic() {
    let mut v = vec![1, 2, 3, 4, 5];
    rotate_left(&mut v, 2);
    assert_eq!(v, vec![3, 4, 5, 1, 2]);
}

#[test]
fn rotate_left_wraps_k() {
    let mut v = vec![1, 2, 3];
    rotate_left(&mut v, 5); // 5 % 3 == 2
    assert_eq!(v, vec![3, 1, 2]);
}

#[test]
fn rotate_left_by_zero_and_by_len() {
    let mut v = vec![1, 2, 3];
    rotate_left(&mut v, 0);
    assert_eq!(v, vec![1, 2, 3]);
    rotate_left(&mut v, 3);
    assert_eq!(v, vec![1, 2, 3]);
}

#[test]
fn rotate_left_empty_no_panic() {
    let mut v: Vec<i32> = vec![];
    rotate_left(&mut v, 4);
    assert!(v.is_empty());
}

#[test]
fn rotate_left_generic_over_t() {
    let mut v = vec!["a".to_string(), "b".to_string(), "c".to_string()];
    rotate_left(&mut v, 1);
    assert_eq!(v, vec!["b", "c", "a"]);
}
