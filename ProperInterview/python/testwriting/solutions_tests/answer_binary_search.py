"""Reference test suite for buggy_binary_search — run explicitly:

    pytest testwriting/solutions_tests/answer_binary_search.py

Expected result: FAILURES. These tests assert the CLAIMED (correct)
behavior, so every failure is a planted bug being caught. Named
answer_*.py so a plain `pytest` never auto-collects it.
"""

import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))

from buggy_binary_search import binary_search


# --- sanity: cases even the buggy version gets right (these pass) --------

def test_sanity_found_in_middle():
    assert binary_search([1, 3, 5, 7, 9], 5) == 2


def test_sanity_absent_small():
    assert binary_search([1, 3, 5, 7, 9], 4) == -1


# --- BUG 1: empty list returns 0 instead of -1 ----------------------------

def test_empty_list_returns_minus_one():
    assert binary_search([], 42) == -1


# --- BUG 2: `while lo < hi` never tests the last remaining index ----------

def test_single_element_found():
    assert binary_search([5], 5) == 0


def test_target_at_last_index():
    assert binary_search([1, 2, 3], 3) == 2


def test_two_elements_second_is_target():
    assert binary_search([1, 3], 3) == 1


def test_every_index_is_findable():
    """The systematic version: search for every element of a known list."""
    data = list(range(0, 40, 3))
    for i, x in enumerate(data):
        assert binary_search(data, x) == i, f"missed index {i}"
