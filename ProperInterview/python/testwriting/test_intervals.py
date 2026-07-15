from buggy_intervals import merge_intervals

internal = [[1, 5], [2, 4]]


def test_touching():
    touching = [[1, 2], [2, 5]]
    assert merge_intervals(touching) == [[1, 5]]
    assert touching == [[1, 2], [2, 5]]


def test_no_merge():
    no_merge = [[1, 2], [3, 5]]
    assert merge_intervals(no_merge) == [[1, 2], [3, 5]]


def test_overlap():
    overlap = [[1, 5], [3, 8]]
    assert merge_intervals(overlap) == [[1, 8]]
    assert overlap == [[1, 5], [3, 8]]


def test_internal():
    assert merge_intervals(internal) == [[1, 5]]


def test_empty():
    assert merge_intervals([]) == []
