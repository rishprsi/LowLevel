use proper_rust::ownership::*;
use std::collections::HashMap;

#[test]
fn shout_basic() {
    assert_eq!(shout("hello"), "HELLO!");
}

#[test]
fn shout_already_upper_and_mixed() {
    assert_eq!(shout("HELLO"), "HELLO!");
    assert_eq!(shout("hElLo, world"), "HELLO, WORLD!");
}

#[test]
fn shout_empty() {
    assert_eq!(shout(""), "!");
}

#[test]
fn shout_does_not_consume_input() {
    let s = String::from("keep me");
    let _ = shout(&s);
    // s is still usable — shout only borrowed it.
    assert_eq!(s, "keep me");
}

#[test]
fn longest_picks_longer() {
    assert_eq!(longest("long string", "short"), "long string");
    assert_eq!(longest("a", "abc"), "abc");
}

#[test]
fn longest_tie_returns_first() {
    assert_eq!(longest("abc", "xyz"), "abc");
    assert_eq!(longest("", ""), "");
}

#[test]
fn longest_result_borrows_from_inputs() {
    let a = String::from("hello there");
    let result;
    {
        let b = String::from("hi");
        result = longest(a.as_str(), b.as_str());
        assert_eq!(result, "hello there");
    }
    // NOTE: using `result` out here would not compile — it may borrow from
    // `b`, which is gone. That's the lifetime contract doing its job.
}

#[test]
fn char_freq_basic() {
    let f = char_freq("aab");
    assert_eq!(f.get(&'a'), Some(&2));
    assert_eq!(f.get(&'b'), Some(&1));
    assert_eq!(f.len(), 2);
}

#[test]
fn char_freq_counts_whitespace_and_unicode() {
    let f = char_freq("a a é");
    assert_eq!(f.get(&'a'), Some(&2));
    assert_eq!(f.get(&' '), Some(&2));
    assert_eq!(f.get(&'é'), Some(&1));
}

#[test]
fn char_freq_empty() {
    assert_eq!(char_freq(""), HashMap::new());
}

#[test]
fn take_evens_basic() {
    assert_eq!(take_evens(vec![1, 2, 3, 4, 5, 6]), vec![2, 4, 6]);
}

#[test]
fn take_evens_preserves_order_handles_zero_and_negatives() {
    assert_eq!(take_evens(vec![0, -3, -2, 7, 8]), vec![0, -2, 8]);
}

#[test]
fn take_evens_empty_and_all_odd() {
    assert_eq!(take_evens(vec![]), Vec::<i32>::new());
    assert_eq!(take_evens(vec![1, 3, 5]), Vec::<i32>::new());
}

#[test]
fn sum_refs_basic() {
    assert_eq!(sum_refs(&[1, 2, 3]), 6);
    assert_eq!(sum_refs(&[]), 0);
}

#[test]
fn sum_refs_no_overflow() {
    // Would overflow i32; must be exact in i64.
    assert_eq!(sum_refs(&[i32::MAX, i32::MAX]), 2 * (i32::MAX as i64));
    assert_eq!(sum_refs(&[i32::MIN, i32::MIN]), 2 * (i32::MIN as i64));
}

#[test]
fn sum_refs_borrows_only() {
    let v = vec![10, 20, 30];
    assert_eq!(sum_refs(&v), 60);
    // v still usable after the call.
    assert_eq!(v.len(), 3);
}
