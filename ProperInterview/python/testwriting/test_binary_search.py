from buggy_binary_search import binary_search


valid = [1, 2, 3, 4, 5, 6, 7]
duplicates = [1, 2, 2, 3, 4, 5, 5]


def test_valid_search():
    assert binary_search(valid, 5) == 4


def test_invalid_search():
    assert binary_search(valid, 10) == -1


def test_duplicates():
    assert binary_search(duplicates, 5) == 5


def test_empty():
    assert binary_search([], 2) == -1
