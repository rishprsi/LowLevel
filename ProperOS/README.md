# ProperOS — OS & concurrency self-test (APUE)

A set of implement-it-yourself systems programming exercises with a test
suite that proves correctness. You implement the stubs in `src/`; the tests
in `tests/` are the spec. The curriculum is the user-space half of APUE
(Stevens & Rago) plus C11 concurrency: file I/O, processes, pipes, signals,
threads, atomics, mmap, and filesystem metadata. Everything builds with
`-Wall -Wextra -Werror -fsanitize=address,undefined`.

Everything builds AND passes on macOS; the one Linux-only module (`procfs`)
compiles everywhere and runtime-skips itself on macOS.

Per-function documentation and background: see [MODULES.md](MODULES.md).

## Workflow (red → green)

1. Pick a module (start with `fileio`).
2. Open `src/<module>.h` — the doc comments are the contract for each function.
3. Implement the functions in `src/<module>.c`, deleting the `TODO(...)` lines.
   - Until you do, the file is a **hard compile error** (that's intentional).
4. Run that module's tests and iterate until green:

   ```sh
   make test_fileio
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
| `make test_<module>` | Build + run just one module, e.g. `make test_signals` |
| `make test-solutions` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make sol_<module>` | Run one module against its reference solution |
| `make clean` | Remove `build/` |

When a check fails it prints a labeled block — the section (function/topic under
test) with a per-section check number, the input expression, and the expected vs
actual value — then the binary exits nonzero:

```
FAIL tests/test_fileio.c:41 [read_all #2]
  input:    (long)n
  expected: 11
  actual:   5
```

The `*_MSG` macro variants (e.g. `CHECK_INT_EQ_MSG(got, want, "fd=%d", fd)`)
append the concrete variable values to the `input:` line in parentheses — useful
when the input is a variable whose value isn't visible in the expression.

Each function (`SECTION`) runs until its **first** failing check: that failure
is printed and the rest of that section is skipped, so every function in the
file still runs but you get exactly one failure block per function (a fuzz loop
shows one block, not thousands). `CTEST_END()` lists every section that failed.

A built-in watchdog guards against hangs: if any check (or the code between
checks) runs longer than 10s, it prints `*** TIMEOUT ... in section: <name>`
and exits, so an infinite loop in your implementation names the function it's
stuck in instead of spinning forever. Raise the limit for a genuinely slow
module with `make test_<m> EXTRA_CFLAGS=-DCTEST_TIMEOUT_SECS=30`. (This
watchdog can't catch the sanitizer hang some sandboxes hit at startup — use
`SAN=undefined` there.)
AddressSanitizer / UBSan will additionally abort with a detailed report on any
out-of-bounds access, leak, or undefined operation.

## Modules

| Module | APUE / topic | Concepts verified |
| --- | --- | --- |
| `fileio` | ch.3 | `read`/`write` partial transfers & EINTR loops, `read_all`/`write_all`, binary-safe file copy, `lseek` size without moving the offset |
| `procs` | ch.8 | `fork` + `execv` + `waitpid`, exit-status decoding (`WIFEXITED`/`WEXITSTATUS`), capturing child stdout with `pipe` + `dup2` |
| `pipes` | ch.15 | building `cmd1 \| cmd2` (and 3-stage) by hand: pipe plumbing, closing unused ends, reaping every child |
| `signals` | ch.10 | `sigaction` (+`SA_RESTART`), blocking/pending signals, SIGCHLD reaping with `WNOHANG` loops, the **self-pipe trick** with `poll` |
| `threads` | ch.11–12 | mutex-protected counter, bounded blocking queue (mutex + two condvars, while-loop waits), minimal thread pool with clean shutdown |
| `atomics` | C11 / phase 4.2 | spinlock over `atomic_flag`, **lock-free SPSC ring buffer** with acquire/release ordering |
| `mmapio` | ch.14 | `mmap` for reading and shared writable mappings, the empty-file (length 0) trap, `msync` |
| `fsmeta` | ch.4 | `opendir`/`readdir` recursion, `lstat` vs `stat`, symlinks not followed, hard-link detection via `st_dev`+`st_ino` |
| `procfs` | Linux `/proc` | parsing `/proc/self/maps` and `/proc/self/status` — **runtime-skipped on macOS** |

## Reference solutions

`solutions/` holds complete implementations and is **git-ignored** — peek only
when stuck. They double as the oracle for `make test-solutions`: if those pass
but your `src/` version fails, the bug is in your implementation, not the test.

## Sanitizers

Tests build with **AddressSanitizer + UndefinedBehaviorSanitizer** by default.
You can change the set with the `SAN` variable:

```sh
make test                 # ASan + UBSan (default)
make test SAN=undefined   # UBSan only (use if ASan misbehaves in a VM/sandbox)
make test SAN=thread      # ThreadSanitizer
make test SAN=            # no sanitizers
```

> Note: in some heavily virtualized/sandboxed environments an ASan-instrumented
> binary can hang at startup. If `make test` seems to freeze, try
> `make test SAN=undefined`. In a normal terminal ASan works fine.

**ThreadSanitizer is the real verifier for the threaded modules.** A data race
can pass a value-equality test by luck; TSan catches the race itself. On a
normal machine, re-run:

```sh
make test_threads SAN=thread
make test_atomics SAN=thread
```

(ASan and TSan are mutually exclusive, which is why TSan is a separate run.)

## Requirements

A C17 compiler with ASan/UBSan/TSan (clang or gcc) and pthreads. On macOS the
default `cc` (clang) works out of the box. Everything passes on macOS; `procfs`
additionally needs Linux for its real (non-skipped) tests.
