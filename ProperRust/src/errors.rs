//! `Result`, custom error enums, `?`, and `Display`.
#![allow(dead_code, unused_variables)]

use std::fmt;

/// Everything that can go wrong when parsing a percentage.
#[derive(Debug, PartialEq)]
pub enum ParseError {
    /// Input was empty (after trimming whitespace).
    Empty,
    /// Input wasn't an integer at all; carries the offending (trimmed) text.
    NotANumber(String),
    /// Input was an integer but outside `0..=100`; carries the value.
    OutOfRange(i64),
}

impl fmt::Display for ParseError {
    /// Human-readable messages — exactly these formats:
    ///
    /// - `Empty` → `empty input`
    /// - `NotANumber(s)` → `not a number: "s"` (the offending string in double quotes)
    /// - `OutOfRange(n)` → `out of range: N`
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        todo!("implement Display for ParseError")
    }
}

/// Parse `s` as a percentage in `0..=100`.
///
/// Steps: trim whitespace; empty → `Err(Empty)`; not parseable as an
/// integer → `Err(NotANumber(trimmed_text.to_string()))`; parseable but
/// outside `0..=100` → `Err(OutOfRange(value))`; otherwise `Ok(value as u8)`.
///
/// - `parse_percent(" 42 ")` → `Ok(42)`
/// - `parse_percent("12x")` → `Err(NotANumber("12x".into()))`
/// - `parse_percent("-1")` → `Err(OutOfRange(-1))`
pub fn parse_percent(s: &str) -> Result<u8, ParseError> {
    todo!("implement parse_percent")
}

/// Parse every item with [`parse_percent`] and sum the results as `u32`.
/// The **first** error encountered is returned (use `?` in a loop, or
/// `collect::<Result<Vec<_>, _>>()`).
///
/// - `sum_percents(&["10", "20"])` → `Ok(30)`
/// - `sum_percents(&[])` → `Ok(0)`
pub fn sum_percents(items: &[&str]) -> Result<u32, ParseError> {
    todo!("implement sum_percents")
}

/// Parse `s` as a percent, falling back to `default` on any error.
/// One-liner with `unwrap_or`.
pub fn parse_or_default(s: &str, default: u8) -> u8 {
    todo!("implement parse_or_default")
}
