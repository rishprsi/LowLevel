//! Traits, trait objects, and generics.
#![allow(dead_code, unused_variables)]

/// Return the largest element of `xs`, or `None` if the slice is empty.
///
/// `T: PartialOrd + Copy` — works for any copyable, comparable type
/// (integers, floats, chars, ...).
pub fn largest<T: PartialOrd + Copy>(xs: &[T]) -> Option<T> {
    todo!("implement largest")
}

/// Something that can describe itself as a human-readable string.
pub trait Describe {
    fn describe(&self) -> String;
}

/// A 2D point. Provided — you implement `Describe` for it below.
pub struct Point {
    pub x: f64,
    pub y: f64,
}

/// A circle. Provided — you implement `Describe` for it below.
pub struct Circle {
    pub radius: f64,
}

impl Describe for Point {
    /// Format: `Point(x, y)` using `{}` (Display) for the coordinates,
    /// e.g. `Point { x: 1.0, y: 2.5 }` → `"Point(1, 2.5)"`.
    fn describe(&self) -> String {
        todo!("implement Describe for Point")
    }
}

impl Describe for Circle {
    /// Format: `Circle(r=RADIUS)` using `{}` for the radius,
    /// e.g. `Circle { radius: 3.0 }` → `"Circle(r=3)"`.
    fn describe(&self) -> String {
        todo!("implement Describe for Circle")
    }
}

/// Call `describe()` on every item, in order. Note the slice of *trait
/// objects* — dynamic dispatch, mixed concrete types in one slice.
pub fn describe_all(items: &[&dyn Describe]) -> Vec<String> {
    todo!("implement describe_all")
}

/// A pair of two values of the same type.
pub struct Pair<T> {
    pub first: T,
    pub second: T,
}

impl<T> Pair<T> {
    pub fn new(first: T, second: T) -> Self {
        Pair { first, second }
    }

    /// Borrow the larger of the two values. On a tie, return `first`.
    ///
    /// Note the `where` clause: `Pair<T>` exists for any `T`, but `larger`
    /// is only available when `T` is comparable.
    pub fn larger(&self) -> &T
    where
        T: PartialOrd,
    {
        todo!("implement Pair::larger")
    }
}
