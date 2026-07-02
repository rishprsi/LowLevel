# ProperRust — module documentation

Per-function documentation and conceptual background for every module. The
doc comments in `src/` remain the authoritative contracts; this file adds the
"why" — which chapter of *The Rust Programming Language* ("The Book") each
module maps to, and which piece of C discipline from the `Proper*` C repos
each concept formalizes and makes compiler-checked. Modules are listed in
the recommended (Makefile) order.

## ownership

**Purpose** — The core ownership toolkit: borrowing vs. moving, `&str` vs.
`String`, explicit lifetimes, and choosing consuming vs. borrowing
signatures.

**Background** — The Book ch.4 (ownership & borrowing) and ch.10.3
(lifetimes). This is the module where Rust formalizes the discipline you
maintain by hand in C: "who frees this buffer, and who is allowed to touch it
meanwhile?" In C those are comments and conventions; in Rust they're the type
system — a move transfers the free obligation, a `&` borrow is a promise not
to outlive or mutate, and the compiler rejects violations instead of leaving
them for ASan. The `&str`/`String` split mirrors the C distinction between a
pointer into someone else's buffer and a `malloc`'d buffer you own; "borrow
in, own out" (`shout`) is the signature shape that replaces C's "caller
provides the output buffer" convention. The explicit lifetime on `longest`
makes visible what a C function returning one of its pointer arguments leaves
implicit — and unchecked: the result can't be used after the shorter-lived
input is gone (no more dangling returns). And `take_evens` consuming its
`Vec` is a *use-after-free made into a compile error*: the caller literally
cannot touch `v` again.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `shout` | Returns `s` uppercased with `'!'` appended | "Borrow in, own out": takes `&str`, returns a fresh `String` |
| `longest` | Returns the longer of two `&str`s (ties → `a`) | The `'a` lifetime pins the result to the shorter-lived input — the checked version of returning a pointer argument in C |
| `char_freq` | Counts occurrences of every `char` in `s` into a `HashMap` | Iterates Unicode scalar values (`.chars()`), not bytes |
| `take_evens` | Consumes `v` and returns only the even elements, in order | Takes `Vec<i32>` by value — the caller can't use it afterwards, by construction |
| `sum_refs` | Sums a borrowed `&[i32]` as `i64` | Widen before summing so `i32::MAX + i32::MAX` is exact — the overflow C silently accepts |

## collections

**Purpose** — Working `Vec` and `HashMap` idioms: in-place mutation, the
entry API, iterator chains, and sorting with compound keys.

**Background** — The Book ch.8 (collections) and ch.13 (iterators). These
two types are the safe replacements for the hand-rolled growable arrays and
hash tables of C: `Vec` is `malloc`/`realloc`/`free` with the resizing logic
and bounds discipline built in, and `HashMap`'s entry API
(`*map.entry(k).or_insert(0) += 1`) collapses C's "lookup, branch on NULL,
insert, update" dance into one expression that can't leave the table
half-updated. The iterator chapter matters because Rust iterators are a
zero-cost abstraction — `filter`/`map`/`collect` chains compile down to the
same loops you'd write by hand, so there's no performance excuse for index
arithmetic. `top_k`'s "count descending, then word ascending" ordering is the
standard compound sort key (`sort_by` with a tuple comparison, or
`sort_by_key` with `Reverse`), and `rotate_left` is a classic in-place
algorithm where the borrow checker quietly rules out the aliasing bugs the C
version invites — while the `k % len` reduction adds the divide-by-zero guard
for the empty vec.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `dedup_sorted` | Removes adjacent duplicates from `v` in place (sorted input ⇒ removes all duplicates) | In-place `&mut Vec` mutation; keep the first of each run |
| `word_count` | Counts whitespace-split, lowercased words into a `HashMap` | Entry API territory; punctuation is *not* stripped — `"hi,"` ≠ `"hi"` |
| `top_k` | Returns up to `k` `(word, count)` pairs: count descending, ties by word ascending | Compound sort key; `k` larger than the map returns everything, `k == 0` returns empty |
| `rotate_left` | Rotates `v` left by `k` in place; `k` may exceed `len` | Reduce with `k % len` — but an empty vec must be a no-op *before* the modulo (divide by zero) |

## linkedlist

**Purpose** — A singly linked list in 100% safe Rust — `Box` + `Option`
nodes, `Option::take`/`map`/`as_deref` manipulation, and a borrowing
iterator.

**Background** — Follows "Learn Rust With Entirely Too Many Linked Lists"
(the second, `Option<Box<Node>>` list); the ingredients are The Book ch.15
(`Box`) and ch.13 (iterators). In C a linked list is `malloc`, `free`, and
pointer surgery where every mistake — leak, double free, use-after-free — is
discovered at runtime, if ever. Here the same structure is built where
ownership is *linear*: each node uniquely owns the next through
`Option<Box<Node<T>>>`, so there is exactly one deallocation path and the
compiler enforces it. The price is that you can't casually detach and reattach
pointers; the idiom that replaces pointer juggling is `Option::take` (steal
the head, leaving `None` — C's "save the pointer before overwriting it," made
atomic) and `Option::map` (transform the stolen node without an explicit
`match`). The borrowing `Iter` shows how a lifetime threads through a data
structure — `as_deref` converts `&Option<Box<Node>>` into `Option<&Node>` so
the iterator holds plain references. The famous lesson at the end: dropping a
100,000-node list can overflow the stack, because the compiler-generated drop
is recursive — the manual `Drop` impl that pops in a loop is the recursion →
iteration conversion every C programmer already knows.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `List::new` | Creates an empty list | `head: None` — the whole list is one `Option` |
| `List::push` | Pushes `elem` onto the front | `Option::take` the old head into the new node's `next` — you can't leave `head` momentarily invalid |
| `List::pop` | Removes and returns the front element (`None` if empty) | `take` + `map`: move the node out, keep its `elem`, relink `next` |
| `List::peek` / `List::peek_mut` | Borrow (mutably) the front element | `as_ref`/`as_mut` to borrow *into* the `Option<Box<...>>` without moving it |
| `List::len` / `List::is_empty` | Element count (O(n) is fine) / emptiness check | Simple traversal — no cached length field to keep in sync |
| `List::into_vec` | Consumes the list, returning elements front-to-back | After `push(1); push(2); push(3)` → `[3, 2, 1]` (it's a stack) |
| `List::iter` / `Iter::next` | Borrowing front-to-back iterator | `as_deref` turns `&Option<Box<Node>>` into `Option<&Node>`; the `'a` lifetime ties items to the list |

**Related:** if the 100k-element drop test blows the stack, write the manual
loop-based `Drop` — the same fix as converting a recursive `free_list()` to a
loop in C.

## traits_generics

**Purpose** — Static and dynamic polymorphism: generic functions with trait
bounds, a hand-written trait with two implementations, trait objects
(`&dyn`), and a `where`-clause method on a generic struct.

**Background** — The Book ch.10 (generics & traits) and ch.18 (trait
objects). C has two polymorphism stories, both unchecked: `void *` + casts
(the compiler can't verify anything) and structs of function pointers
(hand-rolled vtables, as in `qsort`'s comparator or a driver's ops table).
Traits formalize both. A bound like `T: PartialOrd + Copy` is a *checked*
contract — `largest` is monomorphized into specialized code per concrete type
(zero-cost, like a type-safe macro), and calling it with an unordered type is
a compile error rather than nonsense at runtime. A `&dyn Describe` trait
object is the function-pointer-struct pattern with the vtable built and
attached by the compiler: a fat pointer (data + vtable) enabling mixed
concrete types in one slice at the cost of dynamic dispatch —
`describe_all` is exactly this trade. The `where` clause on `Pair::larger`
shows conditional API: `Pair<T>` exists for any `T`, but the comparison
method only exists when `T: PartialOrd` — capability advertised in the
signature instead of a comment saying "only call this if...".

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `largest` | Returns the largest element of `xs`, or `None` if empty | `T: PartialOrd + Copy` bound; `Option` return instead of a sentinel value |
| `Describe for Point` | `describe()` → `"Point(x, y)"` via `{}` formatting | `{}` (Display) prints `1.0` as `1` — the expected strings depend on it |
| `Describe for Circle` | `describe()` → `"Circle(r=RADIUS)"` | Same Display-formatting rule |
| `describe_all` | Calls `describe()` on each `&dyn Describe`, in order | Trait objects = compiler-built vtables; mixed concrete types in one slice |
| `Pair::new` / `Pair::larger` | Construct a pair / borrow the larger value (ties → `first`) | The `where T: PartialOrd` clause makes `larger` exist only for comparable `T` |

## errors

**Purpose** — Error *design*, not just handling: a custom error enum with
data-carrying variants, `Display` for human-readable messages, `?`
propagation, and `unwrap_or` fallbacks.

**Background** — The Book ch.9. This module is the direct answer to `errno`
and C's `-1` return convention. In C, errors are an integer in global state:
the caller can silently ignore them, the value carries no payload (which
input failed? what was the offending text?), and forgetting a check is
invisible until it isn't. `Result<T, E>` fixes all three at the type level —
you cannot get the `T` without confronting the `E` — and an enum error type
carries structured payloads (`NotANumber(String)` keeps the offending text;
`OutOfRange(i64)` keeps the value), replacing the string-formatting-at-fault-
site habit with data the caller can match on. The `?` operator is the
early-return-on-error pattern (`if (rc < 0) return rc;` after every C call)
compressed to one character, and `sum_percents` shows fail-fast composition:
first error wins, either via `?` in a loop or by collecting into
`Result<Vec, E>`. `Display` separates the error's *data* from its
*presentation* — and `parse_or_default` shows the whole ladder collapsing to
one `unwrap_or` when the policy is "any failure means use the default."

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `ParseError` (enum) | `Empty`, `NotANumber(String)`, `OutOfRange(i64)` — everything that can go wrong, with payloads | Variants carry data — richer than any errno integer |
| `Display for ParseError` | Exact messages: `empty input`, `not a number: "..."`, `out of range: N` | The tests match these strings exactly (note the double quotes around the text) |
| `parse_percent` | Trims, then parses `s` as an integer in `0..=100`, returning `u8` or the right `ParseError` | Order matters: empty check, then parse (`NotANumber` keeps the *trimmed* text), then range check |
| `sum_percents` | Parses every item and sums as `u32`; first error wins | `?` in a loop or `collect::<Result<Vec<_>, _>>()` — fail-fast composition |
| `parse_or_default` | Parses `s`, falling back to `default` on any error | One-liner: `parse_percent(s).unwrap_or(default)` |

## threads

**Purpose** — `std::thread` fundamentals: fork/join parallel sum,
`Arc<Mutex<_>>` shared state, and an mpsc channel pipeline.

**Background** — The Book ch.16 ("Fearless Concurrency"). Every function here
has a pthread twin in ProperOS's `threads` module, and the difference is
*who* enforces the rules. In C, "only touch the counter with the mutex held"
and "don't use stack data the thread outlives" are conventions verified by
ThreadSanitizer if you're lucky; in Rust they're the `Send`/`Sync` traits and
the borrow checker — `thread::spawn` requires a `'static` closure, so
capturing by `move` isn't a style choice, it's the only thing that compiles.
`Arc<Mutex<T>>` makes both halves of shared state explicit: `Arc` is shared
ownership (atomically refcounted `shared_ptr`, replacing "who frees this?"),
and the `Mutex` *owns its data* — you can't reach the `T` except through
`lock()`, so the C bug of touching data while forgetting the lock is
unrepresentable. Channels are the ProperOS bounded-queue idea as a library
type, with disconnection built into the type system: when every `Sender` is
dropped, `recv` returns `Err` and the receiver loop ends — which is why
`pipeline_double_then_sum`'s "drop the sender" note exists; a forgotten
sender clone is this world's version of ProperNet's forgotten pipe write end,
and it hangs the same way.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `parallel_sum` | Sums `xs` with exactly `nthreads` threads over contiguous chunks; joins and adds partial sums | `move` closures own their chunks; an empty vec returns 0 without spawning; `unwrap()` the join so worker panics propagate |
| `shared_counter` | `nthreads` threads each do `increments` locked increments; returns exactly `nthreads * increments` | `Arc` shares ownership, `Mutex` guards the data — the compiler-checked version of ProperOS's `mutex_counter_run` |
| `pipeline_double_then_sum` | Two channel stages: double each value, then sum; returns the sum | **Drop the sender** after feeding the first channel, or stage 1's receive loop never ends |

## threadpool

**Purpose** — The classic thread pool from The Book ch.21: boxed `FnOnce`
jobs on an mpsc channel, workers sharing the receiver behind
`Arc<Mutex<Receiver<Job>>>`, graceful shutdown in `Drop`.

**Background** — This is the capstone that ports ProperOS's `tpool_run` into
Rust, and every C design decision becomes a type. A task is
`Box<dyn FnOnce() + Send + 'static>`: heap-allocated because closures are
unsized, `FnOnce` because a job runs exactly once, `Send + 'static` because
it crosses a thread boundary and must own its captures — the compiler-checked
version of the "void pointer + context struct" C task convention. mpsc means
multi-producer, *single*-consumer, so many workers can't each hold the
receiver; wrapping it in `Arc<Mutex<Receiver<Job>>>` makes the workers take
turns — the same shared-work-queue-under-a-lock as the C pool, arrived at by
type-system pressure rather than design discipline. Shutdown is where Rust's
`Drop` replaces C's manual teardown protocol: the C pool needs sentinel
values or a stop flag *and* the discipline to always run that code path; here
`Drop` runs deterministically at scope exit, drops the `Sender` first (every
worker's `recv()` then returns `Err` and its loop ends — no sentinels), and
joins each worker so all submitted jobs finish before `drop` returns. The
"must not hang even for a pool that never ran a job" requirement is the
subtle part: if the sender isn't dropped before joining (the reason for the
`Option<Sender<Job>>` field suggestion), `recv` blocks forever and so does
`drop`.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `ThreadPool::new` | Creates a pool with `n` workers pulling jobs off a shared channel; panics if `n == 0` | The single `Receiver` is shared as `Arc<Mutex<Receiver<Job>>>` — mpsc's *single*-consumer rule forces the lock |
| `ThreadPool::execute` | Boxes the closure and sends it down the job channel | `F: FnOnce() + Send + 'static` — run once, movable across threads, owns its captures |
| `Drop for ThreadPool` | Drops the sender (each worker's `recv()` errors out and its loop ends), then joins every worker | Sender must go first or `drop` deadlocks; store it as `Option<Sender<_>>` so it can be `take`n |

**Related:** this is the Rust port of ProperOS `threads`' `tpool_run` —
compare how sentinel-value shutdown there becomes drop-the-sender here.
