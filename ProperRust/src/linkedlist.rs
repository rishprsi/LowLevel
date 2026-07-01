//! A singly linked list, `Box` + `Option` style — the safe-Rust classic
//! (à la "Learn Rust With Entirely Too Many Linked Lists", the second list).
//!
//! No `unsafe` anywhere. The key tools are `Option::take`, `Option::map`,
//! `as_ref`/`as_mut`, and `as_deref`.
//!
//! One of the tests builds a 100,000-element list and drops it. The
//! compiler-generated recursive drop can overflow the stack on a list that
//! deep — if that test crashes with a stack overflow, write a manual
//! `impl<T> Drop for List<T>` that pops nodes in a loop (the classic
//! lesson from the Linked Lists book).
#![allow(dead_code, unused_variables)]

/// A stack backed by a singly linked list. `push`/`pop`/`peek` all operate
/// on the front (head).
pub struct List<T> {
    head: Link<T>,
}

type Link<T> = Option<Box<Node<T>>>;

struct Node<T> {
    elem: T,
    next: Link<T>,
}

impl<T> List<T> {
    /// Create an empty list.
    pub fn new() -> Self {
        todo!("implement List::new")
    }

    /// Push `elem` onto the front of the list.
    pub fn push(&mut self, elem: T) {
        todo!("implement List::push")
    }

    /// Remove and return the front element, or `None` if the list is empty.
    pub fn pop(&mut self) -> Option<T> {
        todo!("implement List::pop")
    }

    /// Borrow the front element, or `None` if the list is empty.
    pub fn peek(&self) -> Option<&T> {
        todo!("implement List::peek")
    }

    /// Mutably borrow the front element, or `None` if the list is empty.
    pub fn peek_mut(&mut self) -> Option<&mut T> {
        todo!("implement List::peek_mut")
    }

    /// Number of elements in the list. O(n) traversal is fine.
    pub fn len(&self) -> usize {
        todo!("implement List::len")
    }

    /// `true` iff the list has no elements.
    pub fn is_empty(&self) -> bool {
        todo!("implement List::is_empty")
    }

    /// Consume the list, returning its elements front-to-back.
    /// After `push(1); push(2); push(3)` this returns `vec![3, 2, 1]`.
    pub fn into_vec(self) -> Vec<T> {
        todo!("implement List::into_vec")
    }

    /// Iterate over the elements front-to-back by reference.
    /// Hint: `self.head.as_deref()` gives you an `Option<&Node<T>>`.
    pub fn iter(&self) -> Iter<'_, T> {
        todo!("implement List::iter")
    }
}

impl<T> Default for List<T> {
    fn default() -> Self {
        Self::new()
    }
}

/// Borrowing iterator over a [`List`], front-to-back.
pub struct Iter<'a, T> {
    next: Option<&'a Node<T>>,
}

impl<'a, T> Iterator for Iter<'a, T> {
    type Item = &'a T;

    fn next(&mut self) -> Option<&'a T> {
        todo!("implement Iter::next")
    }
}
