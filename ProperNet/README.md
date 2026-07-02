# ProperNet — network programming self-test (Phase 4.1)

A set of implement-it-yourself network programming exercises with a test
suite that proves correctness. You implement the stubs in `src/`; the tests in
`tests/` are the spec — and they exercise **real loopback TCP sockets** on your
machine, with client threads dribbling bytes one at a time, so partial
reads/writes and fd hygiene actually matter. Everything builds with
`-Wall -Wextra -Werror -fsanitize=address,undefined`.

Per-function documentation and background: see [MODULES.md](MODULES.md).

## Workflow (red → green)

1. Pick a module (start with `byteorder`).
2. Open `src/<module>.h` — the doc comments are the contract for each function.
3. Implement the functions in `src/<module>.c`, deleting the `TODO(...)` lines.
   - Until you do, the file is a **hard compile error** (that's intentional).
4. Run that module's tests and iterate until green:

   ```sh
   make test_byteorder
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
| `make test_<module>` | Build + run just one module, e.g. `make test_sockets` |
| `make test-solutions` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make sol_<module>` | Run one module against its reference solution |
| `make clean` | Remove `build/` |

When a check fails it prints a labeled block — the section (function/topic under
test) with a per-section check number, the input expression, and the expected vs
actual value — then the binary exits nonzero:

```
FAIL tests/test_urlparse.c:34 [url_parse #3]
  input:    u.port
  expected: 8080
  actual:   80
```

The `*_MSG` macro variants (e.g. `CHECK_INT_EQ_MSG(got, want, "case=%d", i)`)
add a `context:` line — useful when the failing input isn't visible in the
expression. `CTEST_END()` also lists every section that had a failure.
AddressSanitizer / UBSan will additionally abort with a detailed report on any
out-of-bounds access, leak, or undefined operation.

## Modules

| Module | Concepts verified |
| --- | --- |
| `byteorder` | endianness-independent `htons`/`ntohs`/`htonl`/`ntohl` (no `#ifdef`s!), checked against the real thing |
| `urlparse` | parsing http/https URLs: scheme, host, default ports, path, rejecting malformed input |
| `httparse` | parsing raw HTTP/1.1 request heads from a length-bounded buffer; case-insensitive header lookup; torture-tested with truncated/malformed input |
| `sockets` | `send_all` (partial writes), `recv_until` (reassembly), loopback listen/connect with ephemeral ports — verified over real TCP against a client thread |
| `mux` | single-threaded `poll()`-based multi-client echo server, 3 interleaving concurrent clients |

## Portability note (poll, not epoll)

Everything here is **portable POSIX**: `socket`/`bind`/`listen`/`accept`,
`poll(2)`, pthreads. There is deliberately no `epoll` — it is Linux-only, and
it's left for the Phase 4 HTTP-server project on Linux, where an event loop at
scale is the whole point. `poll` teaches the identical loop shape.

(pthreads are used only by the *tests*, to run clients concurrently with your
server code. On macOS no linker flag is needed; on older Linux toolchains,
`make test LDLIBS=-lpthread`.)

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
make test SAN=            # no sanitizers
```

> Note: in some heavily virtualized/sandboxed environments an ASan-instrumented
> binary can hang at startup. If `make test` seems to freeze, try
> `make test SAN=undefined`. In a normal terminal ASan works fine.

## Requirements

A C17 compiler with ASan/UBSan (clang or gcc) on a POSIX system. On macOS the
default `cc` (clang) works out of the box.
