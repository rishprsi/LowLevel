"""Reference test suite for buggy_slugify — run explicitly:

    pytest testwriting/solutions_tests/answer_slugify.py

Expected result: FAILURES. These tests assert the CLAIMED (correct)
behavior, so every failure is a planted bug being caught. Named
answer_*.py so a plain `pytest` never auto-collects it.
"""

import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))

from buggy_slugify import slugify


# --- sanity: cases even the buggy version gets right (these pass) --------

def test_sanity_simple_title():
    assert slugify("Ubuntu Rocks") == "ubuntu-rocks"


def test_sanity_leading_trailing_stripped():
    assert slugify("  Trimmed  ") == "trimmed"


def test_sanity_explicit_dedup_list():
    used = []
    assert slugify("Post", used) == "post"
    assert slugify("Post", used) == "post-2"
    assert slugify("Post", used) == "post-3"


# --- BUG 1: mutable default argument accumulates across calls --------------

def test_repeated_calls_are_independent():
    assert slugify("Hello") == "hello"
    assert slugify("Hello") == "hello", (
        "second bare call must not remember the first"
    )


# --- BUG 2: runs of separators are not collapsed ---------------------------

def test_consecutive_separators_collapse():
    assert slugify("Hello,  World!") == "hello-world"


def test_punctuation_run_collapses():
    assert slugify("a -- b") == "a-b"
