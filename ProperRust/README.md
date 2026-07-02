# ProperRust — Rust fundamentals self-test

A set of implement-it-yourself Rust exercises with a test suite that proves
correctness. You implement the stubs in `src/`; the integration tests in
`tests/` are the spec. This is the Rust sibling of the `Proper*` C repos
(ProperC etc.) — the side-track for a C programmer learning Rust
(Phase 3.5 / 4.3 of the study guide).

Per-function documentation and background: see [MODULES.md](MODULES.md).

## Red → green, the Rust way

**This repo's "red" differs from the C repos.** In ProperC an unimplemented
stub is a hard *compile error*. That isn't practical in Rust, so here every
stub body is `todo!()`:

- the crate **compiles** out of the box,
- but every test **fails with a panic** (`not yet implemented: ...`) until
  you replace the `todo!()` with a real implementation.

So "red" means *panicking tests*, not compile errors. Same discipline,
different mechanism. (Bonus: the borrow checker itself is part of the test —
plenty of wrong implementations here won't even compile, and that's Rust
doing its job.)

## Which test failed?

Unlike the C repos (where a shared `SECTION` label names the function on each
failing assertion), Rust's test harness already does this for you: every
exercise is its own `#[test] fn`, so `cargo test` prints the failing test
function by name and the exact `assert_eq!`/panic file, line, and column, e.g.

```
---- test_ownership::longest_returns_longer stdout ----
thread 'longest_returns_longer' panicked at tests/test_ownership.rs:21:5:
assertion `left == right` failed
  left: "hi"
 right: "hello"
```

Run a single failing test with `cargo test <name>` (substring match) or a
whole module with `cargo test --test test_<module>`, and add `-- --nocapture`
to see any `println!` output.

## Workflow

1. Pick a module (start with `ownership`).
2. Open `src/<module>.rs` — the doc comments are the contract for each
   function/type.
3. Replace the `todo!()` bodies with real implementations.
4. Run that module's tests and iterate until green:

   ```sh
   cargo test --test test_ownership   # or: make test_ownership
   ```

5. When all modules pass:

   ```sh
   cargo test                         # or: make test
   ```

## Commands

| Command | What it does |
| --- | --- |
| `cargo test` | Run every module's tests against your `src/` implementations |
| `cargo test --test test_<module>` | Run one module, e.g. `cargo test --test test_errors` |
| `make test` | Same as `cargo test` |
| `make test_<module>` | Same as `cargo test --test test_<module>` |
| `./check-solutions.sh` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make test-solutions` | Same as `./check-solutions.sh` |
| `make clean` | `cargo clean` + remove `solutions-check/` |

## Modules

| Module | Rust concepts verified |
| --- | --- |
| `ownership` | moves vs borrows, `&str` vs `String`, explicit lifetimes, consuming vs borrowing APIs |
| `collections` | `Vec` in-place mutation, `HashMap` entry API, iterator chains, sorting with custom keys |
| `linkedlist` | `Box` + `Option` data structures, `Option::take`/`map`/`as_deref`, a borrowing `Iterator` — no `unsafe` |
| `traits_generics` | trait bounds, trait objects (`&dyn`), generic structs, `where` clauses |
| `errors` | custom error enums, `Result` + `?`, `Display`, `unwrap_or` — error *design*, not just handling |
| `threads` | `std::thread::spawn`/`join`, `Send`, `Arc<Mutex<_>>`, mpsc channel pipelines |
| `threadpool` | the classic pool: `Arc<Mutex<Receiver<Job>>>`, boxed `FnOnce` jobs, graceful shutdown in `Drop` |

## Reference solutions

`solutions/` holds complete implementations and is **git-ignored** — peek
only when stuck. Each `solutions/<module>.rs` is a drop-in replacement for
the corresponding `src/<module>.rs`. They double as the oracle for
`./check-solutions.sh`: the script copies the crate to `solutions-check/`,
swaps the solutions in over the stubs, and runs `cargo test` there. If that
passes but your `src/` version fails, the bug is in your implementation,
not the tests.

## Requirements

A Rust toolchain (edition 2021, zero dependencies — builds offline).
If you don't have one:

```sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```
