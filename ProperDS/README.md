# ProperDS — data structures & algorithms self-test

A set of implement-it-yourself C exercises covering classic data structures and
algorithms (Phase 2 of the systems-programming study guide). You implement the
stubs in `src/`; the tests in `tests/` are the spec. Everything builds with
`-Wall -Wextra -Werror -fsanitize=address,undefined`, so memory bugs and
undefined behavior are caught the moment a test exercises them.

## Workflow (red → green)

1. Pick a module (start with `vector`).
2. Open `src/<module>.h` — the doc comments are the contract for each function.
3. Implement the functions in `src/<module>.c`, deleting the `TODO(...)` lines.
   - Until you do, the file is a **hard compile error** (that's intentional).
4. Run that module's tests and iterate until green:

   ```sh
   make test_vector
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
| `make test_<module>` | Build + run just one module, e.g. `make test_avl` |
| `make test-solutions` | Run the suite against the reference solutions (sanity-checks the tests) |
| `make sol_<module>` | Run one module against its reference solution |
| `make clean` | Remove `build/` |

A failing check prints `FAIL <file>:<line>: ... (got X, want Y)` and the binary
exits nonzero. AddressSanitizer / UBSan will additionally abort with a detailed
report on any out-of-bounds access, leak, or undefined operation.

## How the tests verify correctness

Beyond ordinary unit tests with edge cases, this repo leans on two disciplines:

- **Randomized differential testing.** Most modules are fuzzed against a
  dumb-but-obviously-correct *oracle* implemented inside the test file (a flat
  array standing in for a hash table, `qsort` standing in for your sort, a
  linear scan standing in for binary search, Bellman–Ford cross-checking
  Dijkstra, …). Thousands of random operations are applied to both and every
  observable result is compared. All randomness uses a **fixed `srand` seed**,
  so failures are deterministic and reproducible, and each module finishes in
  well under 5 seconds.
- **Invariant checkers.** Stateful structures expose a `*_validate()` function
  (e.g. `avl_validate` checks balance factors, stored heights, *and* BST
  ordering) that the tests call after mutations. When a differential test
  fails, run the validator earlier in the sequence to pinpoint the first
  mutation that broke the structure.

## Modules

| Module | Concepts verified |
| --- | --- |
| `vector` | growable array, amortized doubling (0→4→8…), `memmove` insert/remove |
| `sll` | singly linked list: push front/back, remove first occurrence, in-place reverse |
| `dll` | doubly linked list; validator walks both directions checking `next`/`prev` |
| `intrusive` | Linux-kernel-style intrusive list: `struct list_head`, `container_of` |
| `stackqueue` | array-backed stack + fixed-capacity ring-buffer queue (wraparound) |
| `deque` | growable **circular** deque, push/pop at both ends |
| `hashchain` | separate chaining, FNV-1a, upsert/delete, resize at load factor > 1.0 |
| `hashopen` | open addressing, linear probing, **tombstones**, resize at load > 0.7 |
| `bst` | unbalanced BST: three-case delete, min/max, inorder traversal |
| `avl` | AVL rotations; validator checks balance ∈ {-1,0,1}, heights, ordering |
| `heap` | binary min-heap, bottom-up heapify, heapsort built on it |
| `trie` | lowercase a–z trie: prefix vs whole-word, delete with pruning |
| `unionfind` | union by rank, path compression, component counting |
| `graph` | adjacency list: BFS, DFS (recursive + explicit stack), Kahn's topo sort, cycle detection |
| `shortestpath` | Dijkstra, Bellman–Ford (negative-cycle detection), cross-checked |
| `mst` | Kruskal (private union-find) and Prim, cross-checked on random graphs |
| `sorting` | insertion / merge / quick (median-of-three) / heap / hybrid sorts vs `qsort` |
| `search` | binary search, `lower_bound` / `upper_bound` semantics, fuzzed vs linear scan |
| `strdp` | KMP prefix function & search, edit distance, LIS, coin change, 0/1 knapsack |

## Reference solutions

`solutions/` holds complete implementations and is **git-ignored** — peek only
when stuck. They double as the oracle for `make test-solutions`: if those pass
but your `src/` version fails, the bug is in your implementation, not the test.

## Sanitizers

Tests build with **AddressSanitizer + UndefinedBehaviorSanitizer** by default —
they catch exactly the off-by-one and lifetime bugs these structures invite.
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
