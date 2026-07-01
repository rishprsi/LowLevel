use proper_rust::traits_generics::*;

#[test]
fn largest_ints() {
    assert_eq!(largest(&[1, 5, 3]), Some(5));
    assert_eq!(largest(&[-10, -3, -7]), Some(-3));
}

#[test]
fn largest_single_and_empty() {
    assert_eq!(largest(&[42]), Some(42));
    assert_eq!(largest::<i32>(&[]), None);
}

#[test]
fn largest_first_of_equal_maxes_still_max() {
    assert_eq!(largest(&[2, 9, 9, 1]), Some(9));
}

#[test]
fn largest_works_for_floats_and_chars() {
    assert_eq!(largest(&[1.5, 2.5, 0.5]), Some(2.5));
    assert_eq!(largest(&['a', 'z', 'm']), Some('z'));
}

#[test]
fn describe_point() {
    let p = Point { x: 1.0, y: 2.5 };
    assert_eq!(p.describe(), "Point(1, 2.5)");
}

#[test]
fn describe_circle() {
    let c = Circle { radius: 3.0 };
    assert_eq!(c.describe(), "Circle(r=3)");
    let c = Circle { radius: 0.5 };
    assert_eq!(c.describe(), "Circle(r=0.5)");
}

#[test]
fn describe_all_mixed_trait_objects() {
    let p = Point { x: 0.0, y: 0.0 };
    let c = Circle { radius: 1.0 };
    let items: Vec<&dyn Describe> = vec![&p, &c, &p];
    assert_eq!(
        describe_all(&items),
        vec!["Point(0, 0)", "Circle(r=1)", "Point(0, 0)"]
    );
}

#[test]
fn describe_all_empty() {
    let items: Vec<&dyn Describe> = vec![];
    assert!(describe_all(&items).is_empty());
}

#[test]
fn pair_larger_basic() {
    let p = Pair::new(3, 7);
    assert_eq!(*p.larger(), 7);
    let p = Pair::new(9, 2);
    assert_eq!(*p.larger(), 9);
}

#[test]
fn pair_larger_tie_returns_first() {
    let p = Pair::new(String::from("same"), String::from("same"));
    // On a tie the *first* field must be returned (compare addresses).
    assert!(std::ptr::eq(p.larger(), &p.first));
}

#[test]
fn pair_larger_returns_reference_into_pair() {
    let p = Pair::new(10, 20);
    let r = p.larger();
    assert!(std::ptr::eq(r, &p.second));
}

#[test]
fn pair_exists_for_incomparable_types() {
    // Pair<T> itself needs no bounds — only `larger` does.
    struct NoOrd;
    let _p = Pair::new(NoOrd, NoOrd);
}

#[test]
fn pair_larger_strings() {
    let p = Pair::new(String::from("apple"), String::from("banana"));
    assert_eq!(p.larger(), "banana");
}
