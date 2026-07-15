# python/ — Canonical-style Python drills

Canonical's coding rounds are frequently Python, and their signature format
is **"write the code AND its unit tests"** — so this folder drills both
directions:

- `exercises/` — red→green: the pytest suite is the spec, you implement.
- `testwriting/` — the inverse: the implementation exists but is subtly
  buggy, and **you write the pytest suite** that exposes the bugs.

## Setup

```sh
cd python
python3 -m venv .venv
. .venv/bin/activate
pip install pytest
```

On the Ubuntu VM you can instead use the distro package:

```sh
sudo apt install python3-pytest
```

## Running

```sh
pytest -v                            # all exercise suites
pytest exercises/test_duplicates.py  # one module
```

A plain `pytest` from this directory collects only `exercises/` tests —
the reference answer suites under `testwriting/solutions_tests/` are named
so they are NOT auto-collected (you run those explicitly; see below).

## Drill protocol — exercises/

Each module is a stub (docstring = full spec, body raises
`NotImplementedError`) plus a complete pytest file.

1. Start a timer.
2. Read the docstring in `exercises/<module>.py` — it is the interview
   prompt. **Before implementing, write 2-3 of your own test cases on
   paper** — Canonical asks for tests, so drill the habit even though a
   suite is provided.
3. Implement, then run `pytest exercises/test_<module>.py` until green.
4. Re-drill later with `git checkout -- exercises/<module>.py`.

| Module | The question |
| --- | --- |
| `duplicates` | `find_duplicates(items)` — a literally-reported Canonical question |
| `word_freq` | `top_n_words(text, n)` — counting + tie-breaking |
| `logparse` | parse common-log lines; aggregate by status and path |
| `lru` | `LRUCache` — same contract as the C `lru` module; compare them |

Reference implementations live in `solutions/` (git-ignored) — compare
after, don't peek during.

## Drill protocol — testwriting/

Each `buggy_*.py` module claims to work and has **2-3 subtle planted
bugs**. Your job: write a pytest file that exposes every one.

1. Read the module's docstring (the claimed behavior). Do NOT read
   `ANSWERS.md` yet.
2. Write `test_hunt_<name>.py` (any `test_*` name) with cases you'd expect
   a correct implementation to pass: boundaries, empty inputs, repeated
   calls, input mutation, ties.
3. Run it against the buggy module. Each planted bug you catch is a point.
4. Only then open `ANSWERS.md` and compare against the full bug list. The
   reference suites in `solutions_tests/` catch everything; run one
   explicitly with:

   ```sh
   pytest testwriting/solutions_tests/answer_binary_search.py
   ```

   (Expected result: FAILURES — the suites assert *correct* behavior and
   the modules are buggy. Every failure is a caught bug. Delete your
   `test_hunt_*` files before the next repo-wide `pytest` run, or they'll
   keep failing by design.)

| Module | Claimed behavior |
| --- | --- |
| `buggy_binary_search` | `binary_search(sorted_list, target) -> index or -1` |
| `buggy_intervals` | `merge_intervals(intervals)` — merge overlapping `[start, end]` pairs |
| `buggy_slugify` | `slugify(title)` — URL-safe slug from a title |
