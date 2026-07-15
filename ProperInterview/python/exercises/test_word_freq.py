from word_freq import top_n_words


def test_basic_counts():
    assert top_n_words("the cat and the hat", 2) == [("the", 2), ("and", 1)]


def test_case_insensitive_and_lowercased():
    assert top_n_words("The THE the", 1) == [("the", 3)]


def test_punctuation_is_a_separator():
    assert top_n_words("stop. stop! stop?", 1) == [("stop", 3)]


def test_apostrophe_splits():
    assert top_n_words("don't", 5) == [("don", 1), ("t", 1)]


def test_digits_count_as_word_characters():
    assert top_n_words("v2 v2 go", 2) == [("v2", 2), ("go", 1)]


def test_ties_broken_alphabetically():
    assert top_n_words("b a b a c", 3) == [("a", 2), ("b", 2), ("c", 1)]


def test_tie_between_all_words():
    assert top_n_words("pear apple orange", 3) == [
        ("apple", 1),
        ("orange", 1),
        ("pear", 1),
    ]


def test_n_larger_than_distinct_words():
    assert top_n_words("a b a", 10) == [("a", 2), ("b", 1)]


def test_n_zero_and_negative():
    assert top_n_words("a b c", 0) == []
    assert top_n_words("a b c", -3) == []


def test_empty_text():
    assert top_n_words("", 3) == []
    assert top_n_words("... !!! ---", 3) == []
