# ProperMachine — data representation & the machine (Phase 1.3)

Self-test exercises about what your data actually looks like to the machine:
integer promotion rules, IEEE-754 floats at the bit level, struct layout and
padding — plus a benchmark that makes the cache hierarchy visible. Everything
builds with `-Wall -Wextra -Werror -fsanitize=address,undefined`.

## Two mechanisms

This repo uses **two** kinds of exercise:

### 1. Implement-stubs (as in ProperC/ProperNet)

Functions in `src/<module>.c` are stubbed with `TODO("...")`, which is a
**hard compile error** until you replace it with a real implementation. The
doc comments in `src/<module>.h` are the contract; the tests are the judge.

### 2. Predict-then-verify (new here)

Some knowledge isn't a function you can implement — it's a value you either
can predict or you can't. For these, `src/<module>.c` defines a list of

```c
const int PRED_SOMETHING = TODO_PREDICT;
```

constants. `TODO_PREDICT` is **deliberately declared nowhere**, so every
prediction you haven't filled in is a hard compile error — you cannot "just
run the tests to peek at the answer". Each prediction carries a comment
telling you what to think about (never the answer). Replace `TODO_PREDICT`
with your predicted integer, build, and the tests compute the ground truth
**at runtime** and diff it against you. A wrong prediction fails exactly like
a wrong implementation:

```
FAIL tests/test_promotions.c:29: PRED_NEG1_LT_1U == (neg1 < one_u) ? 1 : 0 (got 1, want 0)
```

Everything you're asked to predict is well-defined C17 (no UB); `layout` is
additionally platform-specific by nature and documents its assumptions
(arm64 / AAPCS64).

## Workflow (red → green)

1. Pick a module (start with `promotions`).
2. Open `src/<module>.h` — it explains the module and (for predictions)
   the rules you need.
3. Fill in every `TODO_PREDICT` / implement every `TODO(...)` stub in
   `src/<module>.c`.
4. Run that module's tests and iterate until green:

   ```sh
   make test_promotions
   ```

5. When all modules pass: `make test`. Then run the measurement drill:
   `make bench`.

## Commands

| Command | What it does |
| --- | --- |
| `make` | Build all test binaries (fails to compile any unfilled module) |
| `make test` | Build + run every module's tests; stops at the first failure |
| `make test_<module>` | Build + run just one module, e.g. `make test_floats` |
| `make test-solutions` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make sol_<module>` | Run one module against its reference solution |
| `make bench` | Build (`-O2`, no sanitizers) + run the cache-hierarchy benchmark |
| `make clean` | Remove `build/` |

## Modules

| Module | Mechanism | Concepts verified |
| --- | --- | --- |
| `promotions` | predict | integer promotion, usual arithmetic conversions, unsigned wraparound, truncation toward zero, `sizeof` surprises (14 predictions) |
| `floats` | predict + implement | decimal fractions in binary, float precision limits, NaN/inf semantics; then `float_bits`/`bits_float` via `memcpy`, `my_isnan` from the bit pattern, sign bit of `-0.0f`, ULP distance |
| `layout` | predict | struct padding and alignment on arm64: member offsets, tail padding, member-ordering waste, nested structs, array stride (13 predictions) |
| `bench` | provided | **not a test** — a measurement drill, see below |

## The bench target (the Phase 2.4 measurement drill)

`bench/bench.c` is fully implemented — nothing to fill in. `make bench`
compiles it with `-O2` and **without sanitizers** (so the timings mean
something) and runs two experiments that do identical work per element but
differ only in access pattern:

1. summing a contiguous array vs. walking one million heap-allocated linked
   list nodes in shuffled order (pointer-chasing the prefetcher can't predict)
2. summing a 4096×4096 int matrix row-major vs. column-major

It prints ns/element and the ratios. There is no pass/fail: the ratio **is**
the cache hierarchy becoming visible. Re-read the numbers after each phase of
the systems curriculum — they don't change, but what you see in them does.

## Reference solutions

`solutions/` holds correctly-filled predictions and complete implementations,
and is **git-ignored** — peek only when stuck. They double as the oracle for
`make test-solutions`: if those pass but your `src/` version fails, the
problem is your prediction/implementation, not the test.

## Sanitizers

Tests build with **AddressSanitizer + UndefinedBehaviorSanitizer** by default.
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

A C17 compiler with ASan/UBSan (clang or gcc). The `layout` module assumes
arm64 (Apple Silicon); everything else is portable. On macOS the default `cc`
(clang) works out of the box.
