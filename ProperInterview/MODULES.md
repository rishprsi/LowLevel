# ProperInterview — module documentation

Per-module background and per-function reference for the timed interview
drills. The doc comments in `src/<module>.h` remain the authoritative
contract for every function — this file adds the *why*: why interviewers
ask this exact question, and the pitfalls the tests are built around.

## strfns

**Target: 15 minutes.**

**Purpose** — The warm-up: the four libc string basics (`my_strlen`,
`my_strcpy`, `my_strcmp`, `my_memcpy`) from a blank file, sanitizer-clean.
This is the study guide's Phase 1 question-bank drill ("in 15 minutes,
clean under ASan/UBSan") in repeatable form.

**Background** — These four are asked not because they're hard but because
they expose habits instantly: off-by-one on the NUL terminator, comparing
`char` values without going through `unsigned char` (`strcmp`'s contract),
forgetting `memcpy` must copy exact bytes with no NUL assumptions. In an
interview this is a two-minute confidence screen — it should be pure muscle
memory, which is why the target is 15 minutes for all four including a
green test run.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `size_t my_strlen(const char *s)` | Length excluding NUL | Pointer-walk idiom `p - s` |
| `char *my_strcpy(char *dst, const char *src)` | Copy including NUL; returns dst | Copy the terminator; save original dst |
| `int my_strcmp(const char *a, const char *b)` | Lexicographic compare | Compare as `unsigned char`; stop at first difference OR NUL |
| `void *my_memcpy(void *dst, const void *src, size_t n)` | Copy exactly n bytes | No NUL logic at all; byte pointers, not `void*` arithmetic |

## cstrings

**Target: 20 minutes.**

**Purpose** — Verifies you can do the standard interview string
manipulations in place, without scratch allocations, and parse integers
without invoking undefined behavior.

**Background** — Interviewers ask string questions because they are short to
state and merciless about details: every one of these is really a test of
loop bounds and edge cases. Reverse-words-in-place is the canonical
"reverse the whole string, then reverse each word" trick — the naive
tokenize-into-a-buffer answer signals you didn't see the O(1)-space
solution. `atoi` is asked precisely because of overflow: `INT_MIN` has a
larger magnitude than `INT_MAX`, so accumulating positively can't represent
`-2147483648`, and checking for overflow *after* the multiply is already
undefined behavior — UBSan turns that mistake into a test failure here. The
overflow check must compare against `INT_MIN / 10` (or equivalent) *before*
the arithmetic. Word counting looks trivial and mostly is — the pitfalls
are whitespace *runs* (state machine, not a split-on-one-space) and
feeding a plain `char` to `isspace()` (UB when the byte is negative; cast
through `unsigned char`).

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `size_t words_reverse(char *s)` | Reverse word order in place; returns word count | Reverse-whole-then-reverse-each-word; empty string is 0 words |
| `int my_atoi(const char *s, int *out)` | Strict parse: 0 ok, -1 malformed, -2 overflow | Check overflow BEFORE the multiply/add; accumulate negative so INT_MIN fits; `*out` untouched on error |
| `size_t str_count_words(const char *s)` | Count whitespace-separated words | In-word/between-words state machine; `isspace((unsigned char)c)` |

Related: `python/exercises/word_freq.py` (the Python cousin of word
counting).

## dynarr

**Target: 30 minutes.**

**Purpose** — Verifies you can build the workhorse dynamic array — the
"implement `std::vector`/Python list" question — with the correct growth
contract and shifting semantics.

**Background** — This is the most common "can you actually manage memory"
warm-up: the amortized-O(1) doubling argument is expected narration, and
the growth policy (0 → 8 → double) is part of the contract the tests check.
The classic realloc trap decides pass/fail: `p = realloc(p, n)` leaks the
original block when realloc fails, so stash the result in a temporary and
leave the array *unchanged* on failure. `insert_at`/`remove_at` shift
overlapping ranges — that's `memmove` territory (or a correctly-directed
loop; the wrong direction clobbers elements as you copy). The randomized
differential test drives ~2000 mixed operations against a dead-simple
shadow array, so an off-by-one that fixed tests miss gets flushed out with
the exact operation number that diverged.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `void da_init(DynArr *da)` | Empty state, no allocation | `len == cap == 0`, `data == NULL` |
| `void da_free(DynArr *da)` | Free and reset to empty | Resetting makes double-free safe by construction |
| `bool da_push(DynArr *da, int x)` | Append, growing 0→8→double | realloc into a temp; unchanged on failure |
| `bool da_pop(DynArr *da, int *out)` | Remove last into `*out` | false on empty, `*out` untouched |
| `int da_get(const DynArr *da, size_t i)` | Element `i` | Precondition `i < len` — no bounds check |
| `void da_set(DynArr *da, size_t i, int x)` | Overwrite element `i` | Precondition `i < len` |
| `bool da_insert_at(DynArr *da, size_t i, int x)` | Insert at `i`, shift right | Overlapping copy — memmove or backward loop; `i == len` is append |
| `void da_remove_at(DynArr *da, size_t i)` | Remove at `i`, shift left | `len - i - 1` elements move — the off-by-one lives here |
| `size_t da_len(const DynArr *da)` | Element count | — |
| `size_t da_cap(const DynArr *da)` | Capacity | — |

Related: `hashmap` (the bucket array), ProperDS `vector` (same shape,
different growth constants).

## hashmap

**Target: 60 minutes.**

**Purpose** — Verifies you can build a complete chaining hash table —
hashing, collision handling, upsert/delete on a linked chain, key
ownership, and load-factor-triggered rehashing.

**Background** — "Implement a hash map" is the standard full-length systems
interview question because it composes everything: an array, linked lists,
string handling, and heap ownership. FNV-1a is the interview-friendly hash
(two lines, good distribution) — the detail worth narrating is the order:
xor the byte *then* multiply; reversing it is FNV-1 and distributes
differently. Deleting from a chain is the pointer-to-pointer exercise —
handling head-of-chain removal without a special case is what the `Entry
**pp` idiom is for. Ownership is where memory bugs hide: `hm_put` must
*copy* the key (the tests deliberately clobber the caller's buffer after
putting), and every copy must be freed exactly once — ASan adjudicates.
Resizing at load factor 1.0 (doubling buckets, relinking every entry into
its new chain) is the part most candidates have never actually written;
note you relink the *existing* nodes rather than reallocating them, and
every pointer into the old bucket array is dead after the move.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `HashMap *hm_create(size_t nbuckets)` | Allocate map with nbuckets chains | `calloc` the bucket array (NULL chains) |
| `void hm_destroy(HashMap *m)` | Free entries, key copies, map | Walk each chain saving `next` before freeing; NULL no-op |
| `bool hm_put(HashMap *m, const char *key, int val)` | Upsert; copies the key | Find-first (upsert must not grow len); resize when len > nbuckets |
| `bool hm_get(const HashMap *m, const char *key, int *out)` | Lookup into `*out` | `*out` untouched on miss; `strcmp`, not pointer compare |
| `bool hm_del(HashMap *m, const char *key)` | Remove + free the entry | Pointer-to-pointer unlink; head-of-chain must work |
| `size_t hm_len(const HashMap *m)` | Entry count | Maintained on put/del, not recounted |

Related: `lru` (uses exactly this table, plus a recency list), ProperDS
`hashchain`.

## lru

**Target: 45 minutes.**

**Purpose** — Verifies the single most classic interview data-structure
question (LeetCode 146): an LRU cache with O(1) `get` and `put`, built as a
hash table + doubly-linked recency list.

**Background** — LRU is beloved by interviewers because the naive answers
are all O(n) and the O(1) answer forces a *composition*: the hash table
finds the node, the doubly-linked list reorders it. The list must be
doubly linked precisely so a node found via the table can be unlinked from
the middle in O(1) — a singly-linked list can't give you the predecessor.
Sentinel head/tail nodes eliminate every null check on link/unlink and are
worth the 30 seconds they take to set up. The behavioral subtleties the
tests pin down: a `get` hit promotes but a miss must *not* change recency;
a `put` of an existing key updates *and* promotes (and must not evict);
eviction removes the least-recent node from *both* structures (forgetting
the hash-table side leaves a dangling pointer — ASan will catch the
use-after-free). Capacity 1 exercises every path through a single node.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `LRU *lru_create(size_t capacity)` | Cache holding ≤ capacity entries | Wire the sentinels to each other on init |
| `void lru_destroy(LRU *c)` | Free nodes, table, cache | Walk the list, stop at the tail sentinel; NULL no-op |
| `int lru_get(LRU *c, int key)` | Value or -1; hit promotes | Miss must not touch recency order |
| `void lru_put(LRU *c, int key, int value)` | Upsert + promote; evict LRU at capacity | Update path must not evict; evict from BOTH list and table |
| `size_t lru_len(const LRU *c)` | Current entry count | ≤ capacity always |

Related: `hashmap` + ProperDS `dll` are the two halves;
`python/exercises/lru.py` is the same contract in Python — drill both and
compare.
