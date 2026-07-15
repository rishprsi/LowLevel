# ProperInterview — timed coding-interview drills

Timed, repeatable interview drills for the Canonical sprint. Unlike the other
Proper* repos (which are learn-it-once self-tests), everything here is meant
to be done **against the clock, repeatedly**, until each module is muscle
memory. The C side covers the classic implement-a-data-structure questions;
[`python/`](python/) covers Canonical's Python coding rounds (including their
signature "write the code AND its unit tests" format); `linux/` — hands-on
Ubuntu VM drills and break/fix labs for the Linux-systems interview.

## The drill protocol

1. Pick a module and note its time target (table below).
2. Restore the module to its blank TODO-stub state and start a timer:

   ```sh
   git checkout -- src/<module>.c
   ```

3. Open `src/<module>.h` — the doc comments are the contract, exactly like a
   written interview prompt. Implement in `src/<module>.c`.
4. **Narrate aloud while you type**, as if the interviewer were watching:
   what invariant you're maintaining, why you chose the loop bounds, what the
   edge cases are. The narration is half the drill.
5. Run the module's tests and iterate until green, then stop the timer:

   ```sh
   make test_<module>
   ```

6. Over target time, or peeked at `solutions/`? Do it again tomorrow.

Until you implement a module, its file is a **hard compile error** (each stub
body is a `TODO(...)` static assert) — that's intentional; the red state is
unmissable.

## Time targets

| Module | Target | What you implement |
| --- | --- | --- |
| `strfns` | 15 min | `my_strlen`/`my_strcpy`/`my_strcmp`/`my_memcpy` — the warm-up four |
| `cstrings` | 20 min | in-place word reversal, overflow-safe `atoi`, word count |
| `dynarr` | 30 min | growable int array: push/pop/get/set/insert/remove |
| `hashmap` | 60 min | chaining string→int hash table with FNV-1a + resize |
| `lru` | 45 min | O(1) LRU cache (hash table + doubly-linked list) |

## Commands

| Command | What it does |
| --- | --- |
| `make` | Build all test binaries (fails to compile any module you haven't implemented) |
| `make test` | Build + run every module's tests; stops at the first failure |
| `make test_<module>` | Build + run just one module, e.g. `make test_lru` |
| `make test-solutions` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make sol_<module>` | Run one module against its reference solution |
| `make list` | Print the module list |
| `make clean` | Remove `build/` |

When a check fails it prints a labeled block — the section (function under
test) with a per-section check number, the input expression, and the expected
vs actual value — then the binary exits nonzero:

```
FAIL tests/test_cstrings.c:85 [str_count_words #1]
  input:    str_count_words("")
  expected: 0
  actual:   1
```

The `*_MSG` macro variants append concrete variable values to the `input:`
line in parentheses — the randomized differential tests use them to tell you
exactly which operation number and key/index diverged.

Each function (`SECTION`) runs until its **first** failing check, then the
rest of that section is skipped — so every function still gets reported, but
a diverging fuzz loop shows one failure block, not thousands. `CTEST_END()`
lists every section that failed.

A built-in watchdog kills any check that runs longer than 10s and names the
section it was stuck in — an infinite loop tells you which function to fix
instead of hanging your drill timer. Raise it for a slow machine with
`make test_<m> EXTRA_CFLAGS=-DCTEST_TIMEOUT_SECS=30`.

## Sanitizers

Tests build with **ASan + UBSan** by default; in an interview, "it passed
but leaks" is a fail, so the tools enforce it here too. Override with the
`SAN` variable:

```sh
make test                 # ASan + UBSan (default)
make test SAN=undefined   # UBSan only (use if ASan misbehaves in a VM/sandbox)
make test SAN=            # no sanitizers
```

## Modules

Per-function documentation, interviewer intent, and the classic pitfalls:
see [MODULES.md](MODULES.md).

## Reference solutions

`solutions/` holds complete implementations and is **git-ignored** — after a
drill, compare your approach against them. They double as the oracle for
`make test-solutions`: if those pass but your `src/` version fails, the bug
is in your implementation, not the test.

## Beyond C

- [`python/`](python/) — Canonical-style Python drills: red→green exercises
  (including the literally-reported find-duplicates question and an LRU you
  can compare 1:1 against the C module) **plus the inverse drill**: buggy
  modules where your job is to write the pytest suite that exposes the
  planted bugs. See [python/README.md](python/README.md).
- [`go/`](go/) — Go refresh drills (`dupes` table-driven, `workerpool` with
  goroutines/channels), always finished with `go test -race`. See
  [go/README.md](go/README.md).
- `linux/` — hands-on Ubuntu VM drills and break/fix labs for the
  Linux-systems interview. See [linux/README.md](linux/README.md).

## Requirements

A C17 compiler with ASan/UBSan (clang or gcc); Python 3 + pytest for
`python/`. On macOS the default `cc` (clang) works out of the box.
