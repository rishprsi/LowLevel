# ProperC — C fundamentals self-test

A set of implement-it-yourself C exercises with a test suite that proves
correctness. You implement the stubs in `src/`; the tests in `tests/` are the
spec. Everything builds with `-Wall -Wextra -Werror -fsanitize=address,undefined`,
so memory bugs and undefined behavior are caught the moment a test exercises them.

## Workflow (red → green)

1. Pick a module (start with `pointers`).
2. Open `src/<module>.h` — the doc comments are the contract for each function.
3. Implement the functions in `src/<module>.c`, deleting the `TODO(...)` lines.
   - Until you do, the file is a **hard compile error** (that's intentional).
4. Run that module's tests and iterate until green:

   ```sh
   make test_pointers
   ```

5. When all modules pass:

   ```sh
   make test
   ```

## Commands

| Command | What it does |
| --- | --- |
| `make` | Build all test binaries (fails to compile any module you haven't implemented) |
| `make test` | Build + run every module's tests; stops at the first failure |
| `make test_<module>` | Build + run just one module, e.g. `make test_bits` |
| `make test-solutions` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make sol_<module>` | Run one module against its reference solution |
| `make clean` | Remove `build/` |

When a check fails it prints a labeled block — the section (function/topic under
test) with a per-section check number, the input expression, and the expected vs
actual value — then the binary exits nonzero:

```
FAIL tests/test_strings.c:12 [my_strlen #2]
  input:    my_strlen("hello")
  expected: 5
  actual:   4
```

The `*_MSG` macro variants (e.g. `CHECK_INT_EQ_MSG(got, want, "seed=%u", seed)`)
append the concrete variable values to the `input:` line in parentheses — useful
when the input is a variable whose value isn't visible in the expression.

Each function (`SECTION`) runs until its **first** failing check: that failure
is printed and the rest of that section is skipped, so every function in the
file still runs but you get exactly one failure block per function (a fuzz loop
shows one block, not thousands). `CTEST_END()` lists every section that failed.
AddressSanitizer / UBSan will additionally abort with a detailed report on any
out-of-bounds access, leak, or undefined operation.

## Modules

| Module | Concepts verified |
| --- | --- |
| `pointers` | pointer arithmetic, arrays, `memcpy` vs `memmove` (overlap), in-place rotate |
| `strings` | C-string traversal/termination, `strncpy` semantics, `strcmp` sign, search |
| `memory` | `malloc`/`realloc`/`free`, ownership, a growable `IntVec` |
| `bits` | bitwise ops, masks, popcount, power-of-two, bit reversal |
| `structs` | data representation: endianness, byte (un)packing (struct layout/padding lives in ProperMachine's `layout`) |
| `funcptr` | function pointers: map / filter / reduce / comparator-based sort |
| `linkedlist` | pointer-to-pointer, reversal, slow/fast pointers, Floyd cycle detection |
| `safety` | **buffer overflow**: bounded string ops (`strlcpy`/`strlcat` style) |
| `ub` | **undefined behavior**: detecting signed overflow, safe shifts, safe loads |

Per-function documentation and background: see [MODULES.md](MODULES.md).

## Reference solutions

`solutions/` holds complete implementations and is **git-ignored** — peek only
when stuck. They double as the oracle for `make test-solutions`: if those pass
but your `src/` version fails, the bug is in your implementation, not the test.

## Sanitizers

Tests build with **AddressSanitizer + UndefinedBehaviorSanitizer** by default —
that's the whole point, since they catch the bugs these exercises are about.
You can change the set with the `SAN` variable:

```sh
make test                 # ASan + UBSan (default)
make test SAN=undefined   # UBSan only (use if ASan misbehaves in a VM/sandbox)
make test SAN=            # no sanitizers
```

> Note: in some heavily virtualized/sandboxed environments an ASan-instrumented
> binary can hang at startup. If `make test` seems to freeze, try
> `make test SAN=undefined`. In a normal terminal ASan works fine.

## Requirements

A C17 compiler with ASan/UBSan (clang or gcc). On macOS the default `cc`
(clang) works out of the box.
