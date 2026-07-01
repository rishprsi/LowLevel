use proper_rust::linkedlist::List;

#[test]
fn new_list_is_empty() {
    let list: List<i32> = List::new();
    assert!(list.is_empty());
    assert_eq!(list.len(), 0);
    assert_eq!(list.peek(), None);
}

#[test]
fn push_pop_lifo() {
    let mut list = List::new();
    list.push(1);
    list.push(2);
    list.push(3);
    assert_eq!(list.pop(), Some(3));
    assert_eq!(list.pop(), Some(2));
    assert_eq!(list.pop(), Some(1));
    assert_eq!(list.pop(), None);
}

#[test]
fn pop_empty_returns_none_repeatedly() {
    let mut list: List<i32> = List::new();
    assert_eq!(list.pop(), None);
    assert_eq!(list.pop(), None);
}

#[test]
fn len_and_is_empty_track_operations() {
    let mut list = List::new();
    assert_eq!(list.len(), 0);
    list.push(10);
    list.push(20);
    assert_eq!(list.len(), 2);
    assert!(!list.is_empty());
    list.pop();
    assert_eq!(list.len(), 1);
    list.pop();
    assert!(list.is_empty());
}

#[test]
fn peek_borrows_front() {
    let mut list = List::new();
    assert_eq!(list.peek(), None);
    list.push(1);
    list.push(2);
    assert_eq!(list.peek(), Some(&2));
    // Peeking doesn't remove.
    assert_eq!(list.len(), 2);
    assert_eq!(list.pop(), Some(2));
}

#[test]
fn peek_mut_allows_mutation() {
    let mut list = List::new();
    list.push(1);
    list.push(2);
    if let Some(front) = list.peek_mut() {
        *front = 42;
    }
    assert_eq!(list.pop(), Some(42));
    assert_eq!(list.pop(), Some(1));
}

#[test]
fn peek_mut_empty_is_none() {
    let mut list: List<i32> = List::new();
    assert!(list.peek_mut().is_none());
}

#[test]
fn into_vec_front_to_back() {
    let mut list = List::new();
    list.push(1);
    list.push(2);
    list.push(3);
    assert_eq!(list.into_vec(), vec![3, 2, 1]);
}

#[test]
fn into_vec_empty() {
    let list: List<i32> = List::new();
    assert_eq!(list.into_vec(), Vec::<i32>::new());
}

#[test]
fn iter_yields_references_front_to_back() {
    let mut list = List::new();
    list.push(1);
    list.push(2);
    list.push(3);
    let collected: Vec<&i32> = list.iter().collect();
    assert_eq!(collected, vec![&3, &2, &1]);
    // List unchanged after iteration.
    assert_eq!(list.len(), 3);
    assert_eq!(list.peek(), Some(&3));
}

#[test]
fn iter_empty() {
    let list: List<i32> = List::new();
    assert_eq!(list.iter().count(), 0);
}

#[test]
fn two_independent_iterators() {
    let mut list = List::new();
    list.push(1);
    list.push(2);
    let mut a = list.iter();
    let mut b = list.iter();
    assert_eq!(a.next(), Some(&2));
    assert_eq!(b.next(), Some(&2));
    assert_eq!(a.next(), Some(&1));
    assert_eq!(a.next(), None);
    assert_eq!(b.next(), Some(&1));
}

#[test]
fn works_with_owned_types() {
    let mut list = List::new();
    list.push(String::from("a"));
    list.push(String::from("b"));
    assert_eq!(list.pop(), Some(String::from("b")));
    assert_eq!(list.into_vec(), vec![String::from("a")]);
}

#[test]
fn long_list_drop_no_stack_overflow() {
    // A naive recursive Drop would blow the stack here; the default
    // Box-chain drop in this design is fine at this size, but an
    // iterative `pop`-based structure must also survive it.
    let mut list = List::new();
    for i in 0..100_000 {
        list.push(i);
    }
    assert_eq!(list.len(), 100_000);
    drop(list);
}
