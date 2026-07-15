"""Reference test suite for buggy_intervals — run explicitly:

    pytest testwriting/solutions_tests/answer_intervals.py

Expected result: FAILURES. These tests assert the CLAIMED (correct)
behavior, so every failure is a planted bug being caught. Named
answer_*.py so a plain `pytest` never auto-collects it.
"""

import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))

from buggy_intervals import merge_intervals


# --- sanity: cases even the buggy version gets right (these pass) --------

def test_sanity_overlapping_merge():
    assert merge_intervals([[1, 4], [2, 6]]) == [[1, 6]]


def test_sanity_disjoint_stay_separate():
    assert merge_intervals([[1, 2], [4, 5]]) == [[1, 2], [4, 5]]


def test_sanity_empty():
    assert merge_intervals([]) == []


# --- BUG 1: touching intervals are not merged ------------------------------

def test_touching_intervals_merge():
    assert merge_intervals([[1, 2], [2, 3]]) == [[1, 3]]


def test_touching_chain_merges_fully():
    assert merge_intervals([[5, 6], [1, 2], [2, 3], [3, 5]]) == [[1, 6]]


# --- BUG 2: the input (and its inner lists) is mutated ----------------------

def test_input_list_order_not_mutated():
    data = [[4, 5], [1, 2]]
    merge_intervals(data)
    assert data == [[4, 5], [1, 2]], "input list was reordered in place"


def test_inner_lists_not_mutated():
    a, b = [1, 4], [2, 6]
    merge_intervals([a, b])
    assert a == [1, 4] and b == [2, 6], "caller's intervals were modified"
