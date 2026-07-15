# lowlevel — self-test suites for the Systems Programming study guide

One repo per phase of the study guide (Obsidian vault: `Study Guides/Systems
Programming`). Each follows the same **red → green** pattern: `src/` holds
stubs you implement (hard compile errors until you do), `tests/` is the spec,
`solutions/` is a git-ignored oracle you can check the suite against
(`make test-solutions`) and peek at only when stuck.

| Repo | Study guide phase | Covers |
| --- | --- | --- |
| `ProperC` | Phase 1 §1.2 — Core C | pointers, strings, memory/ownership, bits, representation, function pointers, linked list, buffer overflow, UB |
| `ProperMachine` | Phase 1 §1.3 — The machine | integer promotions, IEEE-754 floats, struct layout/padding (predict-then-verify), cache-effects benchmark |
| `ProperDS` | Phase 2 — DS & algorithms | the `libds` curriculum: lists → hash tables → AVL → heaps → graphs, sorting, binary search, KMP, DP — with differential oracles + invariant checkers |
| `ProperOS` | Phase 3 + Phase 5 §5.1 (APUE) | file I/O, fork/exec/wait, pipes, signals (self-pipe trick), pthreads, atomics/SPSC, mmap, stat/dirwalk, /proc (Linux) |
| `ProperNet` | Phase 4 §4.1 — Networking | byte order, URL + HTTP/1.1 parsing, sockets with partial I/O, poll-based multiplexing |
| `ProperRust` | Phase 3.5 / 4.3 — Rust side-track | ownership, collections, Box linked list, traits, error design, threads/channels, thread pool (`todo!()` stubs, `cargo test`) |
| `ProperInterview` | Canonical Interview Sprint (vault note 12) | timed interview drills: C (strings, dynamic array, hashmap, LRU), Python + pytest (incl. test-*writing* drills), Go (`dupes`, `workerpool` w/ `-race`), and `linux/` — Ubuntu VM walkthroughs + break/fix scenario labs |

## Universal commands (C repos)

```sh
make test              # run everything (stops at first failing module)
make test_<module>     # iterate on one module
make test-solutions    # sanity-check the suite against the reference answers
make clean
```

Sanitizers (ASan + UBSan) are on by default. In sandboxed/virtualized
environments where ASan binaries hang at startup, use `make test SAN=undefined`.
For threaded modules (`ProperOS`), also run `make test_<m> SAN=thread` on a
normal machine — TSan is the real race detector.

`ProperRust`: `cargo test` / `make test`, oracle via `./check-solutions.sh`.

## What is *not* here

- Phase-level **projects** (shell, allocator, thread pool, HTTP server, …) have
  their own specs and verification plans in the vault's `Projects/` notes.
- Phase 5 **kernel-space** work (PK.1–PK.4) can't be unit-tested from userspace;
  each project note carries its own verification plan (lifecycle scripts,
  KASAN, differential tracing) for the Linux VM.
- gdb/assembly-reading fluency (Phase 1 §1.3–1.4) — drills live in the study
  guide; not honestly automatable.
