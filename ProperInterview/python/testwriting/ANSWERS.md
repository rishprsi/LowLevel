# testwriting — planted bugs (SPOILERS)

**Do the drill first.** Write your own pytest suite against each
`buggy_*.py` module and run it before reading a single line below — the
whole point is practicing the "write the tests" half of Canonical's format.

To check your suite against the reference ones, run them **explicitly**
(they are named `answer_*.py` precisely so a plain `pytest` never
auto-collects them):

```sh
pytest testwriting/solutions_tests/answer_binary_search.py
pytest testwriting/solutions_tests/answer_intervals.py
pytest testwriting/solutions_tests/answer_slugify.py
```

Expected output is **failures**: the suites assert the *claimed* (correct)
behavior, so every failing test is a planted bug being caught. Each file
also includes a few `test_sanity_*` cases that pass even against the buggy
code — a reminder that happy-path tests catch nothing.

---

## buggy_binary_search.py — 2 planted bugs

### Bug 1 — empty list returns `0` instead of `-1`

The `if not sorted_list: return 0` guard claims "found at index 0" for a
list that contains nothing. A test only needs one line:

```python
def test_empty_list_returns_minus_one():
    assert binary_search([], 42) == -1
```

### Bug 2 — `while lo < hi` never examines the last remaining index

When the search space narrows to a single candidate (`lo == hi`) the loop
exits without comparing it, so any target that ends up as "the last one
standing" is reported missing — most visibly a single-element list, or the
final index of the list:

```python
def test_single_element_found():
    assert binary_search([5], 5) == 0

def test_target_at_last_index():
    assert binary_search([1, 2, 3], 3) == 2
```

The systematic version of this test — search for *every* element of a
known list and assert each index — is the one habit worth drilling; it
catches every boundary variant of this bug family at once.

## buggy_intervals.py — 2 planted bugs

### Bug 1 — touching intervals are not merged

The merge condition is `interval[0] < merged[-1][1]` (strict), so
`[1, 2], [2, 3]` — which the contract says merge into `[1, 3]` — stay
separate. Boundary condition, classic off-by-one:

```python
def test_touching_intervals_merge():
    assert merge_intervals([[1, 2], [2, 3]]) == [[1, 3]]
```

### Bug 2 — the input is mutated

Two flavors, one root cause: `intervals.sort()` reorders the caller's
list in place, and `merged.append(interval)` aliases the caller's inner
lists, so `merged[-1][1] = ...` rewrites *their* interval's end. Test by
snapshotting the input and comparing after the call:

```python
def test_input_not_mutated():
    data = [[4, 5], [1, 2]]
    merge_intervals(data)
    assert data == [[4, 5], [1, 2]]

def test_inner_lists_not_mutated():
    a, b = [1, 4], [2, 6]
    merge_intervals([a, b])
    assert a == [1, 4] and b == [2, 6]
```

## buggy_slugify.py — 2 planted bugs

### Bug 1 — mutable default argument accumulates state across calls

`def slugify(title, used=[])` shares ONE list across every call that
doesn't pass its own. The first `slugify("Hello")` works; the second
returns `"hello-2"` because the default list still remembers the first
call. The test is simply calling it twice:

```python
def test_repeated_calls_are_independent():
    assert slugify("Hello") == "hello"
    assert slugify("Hello") == "hello"
```

(This is the single most famous Python gotcha — expect interviewers to
probe whether your tests would catch it.)

### Bug 2 — separator runs are not collapsed

Each non-alphanumeric character is replaced by its own hyphen and only
leading/trailing hyphens are stripped, so `"Hello,  World!"` produces
`"hello---world"` instead of the contractual `"hello-world"`:

```python
def test_consecutive_separators_collapse():
    assert slugify("Hello,  World!") == "hello-world"
```
