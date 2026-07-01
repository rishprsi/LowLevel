use proper_rust::threads::*;

#[test]
fn parallel_sum_basic() {
    let xs: Vec<i64> = (1..=100).collect();
    assert_eq!(parallel_sum(xs, 4), 5050);
}

#[test]
fn parallel_sum_one_thread() {
    assert_eq!(parallel_sum(vec![1, 2, 3], 1), 6);
}

#[test]
fn parallel_sum_more_threads_than_elements() {
    assert_eq!(parallel_sum(vec![5, 7], 8), 12);
}

#[test]
fn parallel_sum_empty() {
    assert_eq!(parallel_sum(vec![], 4), 0);
}

#[test]
fn parallel_sum_uneven_chunks() {
    // 7 elements over 3 threads: chunks of unequal size must all be counted.
    let xs = vec![1, 2, 3, 4, 5, 6, 7];
    assert_eq!(parallel_sum(xs, 3), 28);
}

#[test]
fn parallel_sum_negative_values() {
    assert_eq!(parallel_sum(vec![-5, 5, -10, 10, 3], 2), 3);
}

#[test]
fn parallel_sum_large() {
    let xs: Vec<i64> = (0..1_000_000).collect();
    let expected: i64 = 999_999 * 1_000_000 / 2;
    assert_eq!(parallel_sum(xs, 8), expected);
}

#[test]
fn shared_counter_exact() {
    assert_eq!(shared_counter(4, 1000), 4000);
}

#[test]
fn shared_counter_many_threads_small_work() {
    assert_eq!(shared_counter(16, 100), 1600);
}

#[test]
fn shared_counter_single_thread() {
    assert_eq!(shared_counter(1, 500), 500);
}

#[test]
fn shared_counter_zero_increments() {
    assert_eq!(shared_counter(4, 0), 0);
}

#[test]
fn pipeline_basic() {
    assert_eq!(pipeline_double_then_sum(vec![1, 2, 3]), 12);
}

#[test]
fn pipeline_empty_does_not_hang() {
    // The most common bug: a Sender kept alive so recv() never errors.
    assert_eq!(pipeline_double_then_sum(vec![]), 0);
}

#[test]
fn pipeline_single_and_negatives() {
    assert_eq!(pipeline_double_then_sum(vec![21]), 42);
    assert_eq!(pipeline_double_then_sum(vec![-1, -2, 3]), 0);
}

#[test]
fn pipeline_many_items() {
    let xs: Vec<i64> = (1..=10_000).collect();
    let expected: i64 = 2 * (10_000 * 10_001 / 2);
    assert_eq!(pipeline_double_then_sum(xs), expected);
}
