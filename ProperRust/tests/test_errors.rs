use proper_rust::errors::*;

#[test]
fn parse_percent_valid() {
    assert_eq!(parse_percent("0"), Ok(0));
    assert_eq!(parse_percent("42"), Ok(42));
    assert_eq!(parse_percent("100"), Ok(100));
}

#[test]
fn parse_percent_trims_whitespace() {
    assert_eq!(parse_percent(" 42 "), Ok(42));
    assert_eq!(parse_percent("\t7\n"), Ok(7));
}

#[test]
fn parse_percent_empty() {
    assert_eq!(parse_percent(""), Err(ParseError::Empty));
    assert_eq!(parse_percent("   "), Err(ParseError::Empty));
}

#[test]
fn parse_percent_not_a_number_keeps_offending_text() {
    assert_eq!(
        parse_percent("12x"),
        Err(ParseError::NotANumber("12x".to_string()))
    );
    assert_eq!(
        parse_percent(" abc "),
        Err(ParseError::NotANumber("abc".to_string()))
    );
}

#[test]
fn parse_percent_out_of_range() {
    assert_eq!(parse_percent("101"), Err(ParseError::OutOfRange(101)));
    assert_eq!(parse_percent("-1"), Err(ParseError::OutOfRange(-1)));
    assert_eq!(parse_percent("1000"), Err(ParseError::OutOfRange(1000)));
}

#[test]
fn parse_percent_boundaries() {
    assert_eq!(parse_percent("0"), Ok(0));
    assert_eq!(parse_percent("100"), Ok(100));
    assert_eq!(parse_percent("-0"), Ok(0));
}

#[test]
fn display_messages() {
    assert_eq!(ParseError::Empty.to_string(), "empty input");
    assert_eq!(
        ParseError::NotANumber("wat".to_string()).to_string(),
        "not a number: \"wat\""
    );
    assert_eq!(ParseError::OutOfRange(150).to_string(), "out of range: 150");
    assert_eq!(ParseError::OutOfRange(-5).to_string(), "out of range: -5");
}

#[test]
fn sum_percents_all_valid() {
    assert_eq!(sum_percents(&["10", "20", "30"]), Ok(60));
}

#[test]
fn sum_percents_empty_is_zero() {
    assert_eq!(sum_percents(&[]), Ok(0));
}

#[test]
fn sum_percents_can_exceed_u8() {
    // Four 100s: sum is 400, only representable because the result is u32.
    assert_eq!(sum_percents(&["100", "100", "100", "100"]), Ok(400));
}

#[test]
fn sum_percents_first_error_wins() {
    assert_eq!(
        sum_percents(&["10", "oops", "999"]),
        Err(ParseError::NotANumber("oops".to_string()))
    );
    assert_eq!(
        sum_percents(&["10", "999", "oops"]),
        Err(ParseError::OutOfRange(999))
    );
    assert_eq!(sum_percents(&["", "50"]), Err(ParseError::Empty));
}

#[test]
fn parse_or_default_falls_back() {
    assert_eq!(parse_or_default("42", 7), 42);
    assert_eq!(parse_or_default("nope", 7), 7);
    assert_eq!(parse_or_default("", 99), 99);
    assert_eq!(parse_or_default("200", 50), 50);
}
