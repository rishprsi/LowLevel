from duplicates import find_duplicates


def test_no_duplicates():
    assert find_duplicates([1, 2, 3]) == []


def test_empty():
    assert find_duplicates([]) == []


def test_single_duplicate():
    assert find_duplicates([1, 2, 1]) == [1]


def test_each_duplicate_reported_once():
    assert find_duplicates([5, 5, 5, 5]) == [5]


def test_order_is_first_repeat_not_first_occurrence():
    # 1 occurs first, but 2 REPEATS first, so 2 leads.
    assert find_duplicates([1, 2, 2, 1]) == [2, 1]


def test_mixed():
    assert find_duplicates([1, 2, 3, 2, 1, 2]) == [2, 1]


def test_strings():
    assert find_duplicates(["a", "b", "a", "c", "b"]) == ["a", "b"]


def test_works_on_any_iterable():
    assert find_duplicates(iter([1, 1, 2])) == [1]


def test_input_not_mutated():
    data = [3, 1, 3, 1]
    find_duplicates(data)
    assert data == [3, 1, 3, 1]
