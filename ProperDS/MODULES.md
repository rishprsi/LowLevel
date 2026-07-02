# Module documentation

Background and per-function notes for every module in the repo. The doc
comments in `src/<module>.h` are the *contract* — exact preconditions, growth
policies, return conventions. This file adds what the headers deliberately
leave out: how each structure behaves at the memory level, why the tests probe
what they probe, and the classic ways each implementation goes wrong.

Every module is verified two ways: **randomized differential testing** against
a dumb-but-obviously-correct oracle (fixed seed, so failures reproduce), and —
for stateful structures — a **`*_validate()` invariant checker** the tests
call after mutations. When a differential test fails, calling the validator
earlier in the operation sequence pinpoints the first mutation that broke the
structure. Read this file alongside the header, not instead of it.

## vector

**Purpose** — A growable array of `int` with amortized-O(1) append and
`memmove`-based insert/remove at arbitrary indices.

**Background** — The dynamic array is the workhorse of practical programming
because it is the *cache-friendliest* container there is: elements sit
contiguously, so iteration streams through memory a cache line at a time and
the hardware prefetcher does the rest. The amortized argument is the classic
one — doubling capacity means each element is copied O(1) times on average,
even though any individual push can trigger an O(n) `realloc`-and-copy. The
growth policy here (0 → 4 → 8 → …) is part of the contract and the tests check
it. `insert_at`/`remove_at` are O(n) because of the shift, which is why
`memmove` (not `memcpy`) is the right tool: source and destination overlap,
and `memcpy` on overlapping ranges is undefined behavior — exactly the kind
of bug UBSan exists to catch. Common pitfalls: forgetting that `realloc` can
move the buffer (stale pointers), off-by-one in the shift bounds, and leaking
the old buffer on growth. Interview-wise, "how does a dynamic array achieve
amortized O(1)?" is a warm-up question everywhere; being able to *derive* the
amortization rather than recite it is the difference.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `vector_init` / `vector_free` | Set up / tear down the empty state (`len = cap = 0`, `data = NULL`) | `free` must reset the struct so double-free is impossible |
| `vector_push` | Append at the end, growing 0→4 then doubling | Amortized O(1); check the growth *before* writing |
| `vector_pop` | Remove and return the last element | Precondition `len > 0` — no shrinking required |
| `vector_get` / `vector_set` | Read / overwrite index `i` | Preconditions, not bounds checks — ASan catches violations |
| `vector_insert_at` | Insert so `x` lands at index `i`, shifting `[i, len)` right | `i == len` is a legal append; overlapping copy needs `memmove` |
| `vector_remove_at` | Remove index `i`, shifting `(i, len)` left | Shift count is `len - i - 1`, a classic off-by-one site |
| `vector_len` / `vector_cap` | Element count / current capacity | Capacity is observable — the tests verify the doubling policy |
| `vector_validate` | Checks `len <= cap` and `data == NULL` exactly when `cap == 0` | The NULL↔cap==0 pairing catches half-initialized states |

Related: `stackqueue`'s Stack and `heap` are both built on this same growable
array; `deque` is the circular variant.

## sll

**Purpose** — A singly linked list of `int` where the list *is* the head
pointer: push front/back, find, remove-first-occurrence, and in-place
reversal.

**Background** — Linked lists are pedagogically great and practically awful:
every node is a separate heap allocation, so traversal is a chain of dependent
pointer loads that defeats the prefetcher — on modern hardware a vector beats
a list for almost every workload despite the list's O(1) splice. What lists
*are* still good for is teaching pointer discipline, and this module's design
choice is the important lesson: functions that can change which node is first
take `SllNode **head`. That pointer-to-pointer idiom (or equivalently,
iterating with a `SllNode **` "cursor" to the next-field you might rewrite)
eliminates the special cases for empty lists and head removal that plague
naive implementations — Linus Torvalds' famous "good taste" example is exactly
this. In-place reversal (three pointers: prev/cur/next) is one of the most
asked interview questions in existence; the trap is losing the rest of the
list by overwriting `next` before saving it. `sll_free` must not use a node
after freeing it — save `next` first, or ASan will let you know.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `sll_push_front` | Allocate a node, link at the front | On allocation failure the list must be *unchanged* |
| `sll_push_back` | Allocate a node, link at the back | O(n) walk to the tail; empty list means writing `*head` itself |
| `sll_find` | First node with matching value, or NULL | Plain read-only walk |
| `sll_remove_value` | Unlink + free the *first* match | The `**head` idiom makes head removal a non-special case |
| `sll_reverse` | Reverse in place by relinking | No allocation, no value copying; save `next` before rewriting it |
| `sll_length` | Node count | — |
| `sll_free` | Free every node, set `*head = NULL` | Use-after-free if you read `n->next` after `free(n)` |

Related: `dll` adds back-links and a validator; `intrusive` inverts the
ownership model entirely.

## dll

**Purpose** — A doubly linked list with head/tail pointers and a cached
length, supporting O(1) push/pop at both ends.

**Background** — The doubly linked list doubles the pointer overhead of `sll`
(two pointers per node, plus the same cache-hostile allocation pattern) in
exchange for O(1) removal *given a node pointer* and O(1) operations at both
ends — which is why it underlies LRU caches (hash map for lookup + DLL for
recency order) and every textbook deque-of-nodes. The implementation hazard is
symmetry: every link operation must update *four* pointers (`prev`/`next` on
both neighbors), and it's easy to write code that works walking forward but
has a broken `prev` chain that nothing notices — until you pop from the tail.
That is precisely why `dll_validate` walks the list in *both* directions and
cross-checks `n->next->prev == n` at every step: a forward-only validator
would pass a half-linked list. The empty↔one-element transitions (where head
and tail must become equal or NULL together) are where most bugs live. Keeping
`len` cached is a small contract of its own: every mutation must maintain it,
and the validator counts nodes to catch drift.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `dll_init` / `dll_free` | Empty state setup / free-all-and-reset | Empty means `head == tail == NULL` *and* `len == 0` |
| `dll_push_front` / `dll_push_back` | O(1) insert at either end | First insert must set both head and tail |
| `dll_pop_front` / `dll_pop_back` | O(1) remove at either end into `*out` | Last removal must NULL both head and tail; `*out` untouched when empty |
| `dll_length` | Cached count | Must stay in sync with actual node count |
| `dll_validate` | Walks forward from head *and* backward from tail; checks `head->prev == NULL`, `tail->next == NULL`, mutual `next`/`prev` consistency, and that both walks visit exactly `len` nodes | The backward walk is what catches broken `prev` chains |

Related: `sll` is the single-linked version; `intrusive` is a circular DLL
with a sentinel and no ownership.

## intrusive

**Purpose** — A Linux-kernel-style intrusive circular doubly linked list:
`struct list_head` embedded in *your* struct, recovered with
`container_of()`; the list never allocates.

**Background** — This is the real-world list. Linux's `include/linux/list.h`
uses exactly this design for run queues, timer lists, and thousands of other
kernel structures, and the reasons hold outside the kernel too: zero
allocations (the links live inside objects that already exist), one object can
sit on several lists at once (embed several `list_head`s), and removal is O(1)
with no lookup because you already hold the node. The circular-with-sentinel
layout is the elegant part: the empty list is a head pointing at itself, so
insert and delete have *no NULL checks and no special cases* — every node,
including the sentinel, always has valid `next` and `prev`. The magic trick is
`container_of`, which subtracts `offsetof(type, member)` from the member
pointer to recover the containing struct — pure pointer arithmetic, resolved
at compile time. The classic pitfall is captured in `list_del`'s contract:
after unlinking, the entry's own pointers are left dangling, and reusing it
without re-initialization corrupts whatever list it thinks it's still on.
Iteration compares against the sentinel's *address* (`p != &todo`), not
against NULL — forgetting that is the standard first-timer bug.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `LIST_INIT` (macro) | Static initializer: head points at itself | Empty ≠ NULL; empty = self-referential sentinel |
| `container_of` (macro) | Recover the embedding struct from a member pointer | `(char *)` cast + `offsetof` — the whole intrusive trick in one line |
| `list_head_init` | Runtime equivalent of `LIST_INIT` | Needed for heap- or array-allocated heads |
| `list_add` | Insert immediately after `head` (front) | Four pointer writes; order matters |
| `list_add_tail` | Insert immediately before `head` (tail) | Same op viewed from `head->prev` |
| `list_del` | Unlink an entry from whatever list holds it | Entry's own pointers left dangling — re-init before reuse |
| `list_empty` | True iff only the sentinel remains | Check is `head->next == head` |

Related: `dll` is the owning, non-circular cousin — comparing the two shows
what the sentinel buys you.

## stackqueue

**Purpose** — Two array adapters: a growable LIFO stack and a
*fixed-capacity* ring-buffer FIFO queue.

**Background** — Both adapters demonstrate that "stack" and "queue" are
access disciplines, not data structures: the same flat array serves both. The
stack is just `vector_push`/`vector_pop` renamed, and inherits the array's
cache behavior — the hot end stays in L1. The ring buffer is the interesting
half. A naive array queue that dequeues by shifting is O(n); the ring buffer
makes both ends O(1) by letting the logical sequence wrap around the physical
array: element `i` lives at `data[(head + i) % cap]`. Ring buffers are
everywhere real systems need bounded FIFO — kernel I/O rings (io_uring),
network card descriptor rings, audio buffers, lock-free SPSC queues — because
fixed capacity means no allocation on the hot path and predictable memory.
The classic design question is distinguishing full from empty: this module
stores `len` explicitly, which sidesteps the ambiguity (the alternative —
head/tail pointers only — must waste a slot or carry a flag, a favorite
interview follow-up). The classic bug is an index that reaches `cap` without
wrapping; ASan catches the resulting out-of-bounds write immediately.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `stack_init` / `stack_free` | Empty state / free and reset | Same 0→4→doubling growth as `vector` |
| `stack_push` | Push on top, growing as needed | Amortized O(1) |
| `stack_pop` / `stack_peek` | Remove / read the top into `*out` | Return `false` (out untouched) when empty — no precondition here |
| `stack_is_empty` | Emptiness test | — |
| `queue_init` / `queue_free` | Allocate exactly `cap` slots / free | Capacity is fixed forever; `cap > 0` is a precondition |
| `queue_enqueue` | Append at the tail | Returns `false` if full — the queue never grows |
| `queue_dequeue` | Remove the oldest element into `*out` | Advance head with wraparound: `(head + 1) % cap` |
| `queue_is_full` / `queue_is_empty` | Capacity tests | Trivial because `len` is stored explicitly |

Related: `deque` is what you get when the ring buffer learns to grow;
`graph`'s BFS needs a FIFO queue just like this one.

## deque

**Purpose** — A growable circular deque: O(1) push/pop at both ends, backed
by a wrapping array that doubles when full.

**Background** — The deque composes the two ideas from `stackqueue`: ring
arithmetic for O(1) at both ends, plus the vector's doubling for unbounded
growth. The step that trips people up is the grow: after doubling `cap`, the
elements must be *re-linearized* — copied out of their wrapped positions into
the new buffer so logical index 0 sits at physical index 0 again (the header
makes this part of the contract). Simply `realloc`-ing in place is wrong: if
the live elements wrap around the end of the old buffer, the two segments
end up separated by a gap of uninitialized slots. `push_front` needs modular
*decrement* — `head = (head + cap - 1) % cap`, because `head` is unsigned and
`(head - 1) % cap` underflows to a huge number when `head == 0`. That
unsigned-underflow trap is one of C's most reliable bug generators, and UBSan
won't help (unsigned wraparound is defined behavior — it's just *wrong*
here). Real deques (e.g. C++ `std::deque`) use chunked blocks instead of one
ring to avoid the big copy, but the single-ring version is what interviews
mean by "implement a deque."

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `deque_init` / `deque_free` | Empty state / free and reset | Empty: `data = NULL`, `cap = head = len = 0` |
| `deque_push_front` | Insert as new first element | Modular decrement of unsigned `head` — add `cap` before subtracting |
| `deque_push_back` | Insert as new last element | Writes at `(head + len) % cap` |
| `deque_pop_front` / `deque_pop_back` | Remove either end into `*out` | Return `false` (out untouched) when empty |
| `deque_len` | Element count | — |
| `deque_validate` | Checks `len <= cap`, `head < cap` whenever `cap > 0`, `data == NULL` exactly when `cap == 0` | Catches un-wrapped heads and inconsistent grow logic |

Related: `stackqueue` supplies both ingredients (growable array, ring
buffer); a deque can serve as both a stack and a queue.

## hashchain

**Purpose** — A separate-chaining hash table mapping string keys to `int`:
FNV-1a hashing, upsert/get/delete, and doubling the bucket array past load
factor 1.0.

**Background** — Separate chaining is the forgiving hash table: each bucket
heads a linked list, so collisions degrade gracefully and deletion is a
plain list unlink — no tombstones, no probe-sequence reasoning. The cost is
memory-level: every entry is a separate allocation and every lookup after
the bucket index is a pointer chase, so a chained table has strictly worse
cache behavior than open addressing at the same load factor (this is the
comparison `hashopen` exists to make). Expected O(1) operations depend on two
things the contract pins down: a hash that spreads keys (FNV-1a — XOR the
byte *then* multiply; reversing the order is FNV-1 and produces different
values the tests will reject) and resizing before chains get long (here,
doubling when `len > nbuckets`). Rehashing must recompute every entry's
bucket from its hash — carrying old indices over is the classic resize bug,
and `hashchain_validate` exists to catch exactly that: an entry sitting in a
bucket its hash doesn't map to. The other contract detail worth respecting
is key ownership: the table stores its *own copy* of each key (`strdup`),
because the caller's string may die; forgetting to free those copies is the
leak ASan will find.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `hashchain_fnv1a` | The contract hash: FNV-1a, 64-bit | XOR byte then multiply by the FNV prime — order matters |
| `hashchain_init` | Allocate the initial 8-bucket table | Buckets start NULL |
| `hashchain_free` | Free entries, owned keys, and bucket array | Safe to call twice — reset the struct |
| `hashchain_put` | Upsert: insert or overwrite | Resize when `len > nbuckets` *after* insert; rehash every entry |
| `hashchain_get` | Value lookup into `*out` | `strcmp` on the chain, not pointer compare |
| `hashchain_delete` | Remove a key | List unlink — the easy deletion that chaining buys you |
| `hashchain_len` | Number of distinct keys | Upsert of an existing key must *not* bump it |
| `hashchain_validate` | Checks every entry sits in the bucket its hash demands, and total chain entries equal `len` | The misplaced-entry check catches broken rehashing |

Related: `hashopen` solves the same problem with opposite trade-offs — read
the two backgrounds together.

## hashopen

**Purpose** — An open-addressing hash table with linear probing and
tombstones, mapping string keys to `int`; resizes past load factor 0.7.

**Background** — Open addressing stores everything in one flat slot array —
no per-entry allocation, no pointer chasing — so a probe sequence walks
*consecutive* memory and often resolves within a single cache line. That is
why high-performance tables (Google's Swiss tables, Rust's hashbrown, Python's
dict) are open-addressed, despite the two complications this module forces
you to face. First, clustering: linear probing forms runs of occupied slots
that grow superlinearly as load rises, which is why the resize threshold is
0.7 rather than chaining's 1.0. Second — the heart of the module —
**deletion**. You cannot simply mark a deleted slot EMPTY: any key that was
inserted *past* that slot after a collision becomes unreachable, because
probes stop at EMPTY. The fix is the tombstone: a third state that lookups
probe *through* but insertion may recycle. Tombstones then cause their own
disease — a table can be nearly empty yet probe like a full one — so the
resize trigger counts `occupied + tombstones` (`used`), and rehashing drops
all tombstones. Getting the three-state probe loop right (stop at EMPTY,
continue past TOMBSTONE, remember the first tombstone for recycling) is the
whole exercise, and it's a startlingly common senior-interview question
because so few people have actually written it.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `hashopen_fnv1a` | Same FNV-1a 64 contract hash as `hashchain` | Home slot is `hash % nslots` |
| `hashopen_init` | Allocate exactly 16 slots, all `HO_EMPTY` | Zeroed memory conveniently equals `HO_EMPTY = 0` |
| `hashopen_free` | Free all owned keys and the slot array | Only `HO_OCCUPIED` slots own a key |
| `hashopen_put` | Upsert with linear probing | May recycle the first tombstone on the probe path; resize check uses `used`, not `len` |
| `hashopen_get` | Lookup: probe until match or `HO_EMPTY` | Must probe *through* tombstones — stopping at one loses keys |
| `hashopen_delete` | Mark the slot `HO_TOMBSTONE`, free the key, `len--` | `used` stays the same — the tombstone still burns a slot |
| `hashopen_len` | Live (occupied) entries | `len` and `used` diverge exactly by the tombstone count |
| `hashopen_validate` | Checks occupied count == `len`, occupied+tombstones == `used`, `used` within the 0.7 load factor, and every live key reachable by probing from its home slot | The reachability check is what catches stop-at-tombstone bugs |

Related: `hashchain` is the pointer-chasing alternative; the differential
test drives both against the same flat-array oracle.

## bst

**Purpose** — An unbalanced binary search tree of unique `int` keys:
insert, contains, three-case remove, min/max, and inorder traversal.

**Background** — The BST is the idea that makes every ordered container
work: left subtree smaller, right subtree larger, so search is a root-to-leaf
walk. Unbalanced, that walk is O(h) where h can be anything from log n
(random insertion order) to n (sorted insertion order — insert 1,2,3,… and
you've built a linked list with extra steps; the tests do exactly this). The
module's real content is **deletion**, the operation textbooks gloss over
and interviews love: a leaf just disappears; a one-child node splices its
child up into its place; a two-child node can't be removed directly, so you
overwrite its key with the *inorder successor* (minimum of the right
subtree) and then delete that successor — which is guaranteed to have at
most one child, reducing case 3 to case 1 or 2. The recurring implementation
bug is losing the parent link: you find the node to delete but can't rewire
the pointer that points *at* it. The clean fixes are recursion that returns
the new subtree root, or iterating with a `BstNode **` — the same
pointer-to-pointer idiom as `sll`. Memory-level, a pointer-based BST has the
same problem as a linked list — every step is a dependent load — which is
why real databases use B-trees (fat nodes sized to cache lines/pages)
instead.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `bst_init` / `bst_free` | Empty tree / recursive free-all | Free children before the node (postorder) |
| `bst_insert` | Insert a unique key | Duplicates rejected: return `false`, change nothing |
| `bst_contains` | Membership walk | O(h) — h may be n on adversarial input |
| `bst_remove` | Three-case delete | Two-child case: swap in the inorder successor, then delete it from the right subtree |
| `bst_min` / `bst_max` | Leftmost / rightmost key into `*out` | `false` on empty tree, `*out` untouched |
| `bst_size` | Key count | Maintained on insert/remove, verified by the validator |
| `bst_inorder` | Fill `out[]` ascending | Inorder of a valid BST is sorted — the test's main oracle |
| `bst_validate` | Checks the ordering invariant everywhere plus node count == `size` | Must check against *range bounds*, not just each node vs. its children — the classic broken validator only compares parent/child |

Related: `avl` is this tree plus the discipline to stay balanced — implement
`bst` first.

## avl

**Purpose** — A self-balancing AVL tree: BST operations plus rotations that
keep every node's balance factor in {-1, 0, +1}, with cached heights.

**Background** — The AVL tree (1962, the first self-balancing BST) enforces
the strictest practical balance invariant, guaranteeing height ≤ ~1.44 log n
— tighter than red-black trees, so lookups are faster but insertions rotate
more. The mechanics: each node caches its height; after an insert or remove,
walk back up recomputing heights, and where the balance factor hits ±2,
rotate. Four cases — LL and RR need a single rotation, LR and RL need a
double (rotate the child first to turn the "zig-zag" into a straight line,
then rotate the node). The two ways implementations rot: forgetting that a
rotation changes heights and must update the *rotated pair* in the right
order (child last… no — pivot's new children first, then the pivot), and
assuming one rotation fixes everything after a *removal* — deletion can
shorten a subtree and require rebalancing at every level up to the root,
unlike insertion which needs at most one rotation. This is why
`avl_validate` recomputes every height from scratch: a tree can look
balanced while its cached heights lie, and lying heights make future balance
decisions wrong in ways that only surface hundreds of operations later —
exactly the failure mode differential testing plus a validator is designed
to localize. AVL rotations remain a canonical hard-mode interview topic.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `avl_init` / `avl_free` | Empty tree / free-all | Same as `bst` |
| `avl_insert` | Insert unique key, rebalance on the way up | At most one (possibly double) rotation restores balance after insert |
| `avl_contains` | Membership walk | Guaranteed O(log n) now |
| `avl_remove` | Three-case delete plus rebalancing | May need rotations at *every* level back to the root |
| `avl_height` | Whole-tree height (0 if empty) | Height convention: leaf = 1, empty = 0 — off-by-one here corrupts every balance factor |
| `avl_size` | Key count | — |
| `avl_inorder` | Fill `out[]` ascending | Same sorted-output oracle as `bst` |
| `avl_validate` | Recursively verifies BST ordering, every cached height against a from-scratch recomputation, every balance factor in {-1,0,+1}, and node count == `size` | The height recomputation catches stale caches that "look" balanced |

Related: `bst` supplies the skeleton (insert/remove structure, inorder);
only the rebalancing is new.

## heap

**Purpose** — A binary min-heap in a growable array, plus O(n) bottom-up
heapify from an array and a heapsort built on top.

**Background** — The binary heap is the array trick that makes priority
queues cheap: a complete binary tree flattened so node `i`'s children are
`2i+1`/`2i+2` and its parent is `(i-1)/2` — no pointers, no allocation per
element, decent locality near the root where the action is. Push is
sift-up, pop-min is "move the last element to the root and sift down"; both
O(log n). The contract's key demand is that `heap_from_array` use
**bottom-up heapify** — sift-down from the last internal node `(n/2)-1` back
to the root — which is O(n), not the O(n log n) of n pushes. The proof is a
nice interview moment: most nodes are near the bottom and sift almost
nowhere, and the cost sum telescopes to O(n). Sift-down has a classic bug:
you must swap with the *smaller* child (in a min-heap), and you must check
that the right child exists before comparing — reading `data[2i+2]` when
only the left child exists is exactly the off-by-one ASan is here for. Note
the heap property alone says nothing about order between siblings or across
subtrees — a common misconception `heap_validate` implicitly documents by
checking only parent ≤ children. The function is named `heap_sort` (not
`heapsort`) because macOS's `<stdlib.h>` already declares a BSD `heapsort` —
a small real-world lesson in namespace collisions with libc.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `heap_init` / `heap_free` | Empty state / free and reset | Same growable-array skeleton as `vector` |
| `heap_push` | Insert with sift-up | Compare against parent `(i-1)/2` until the property holds |
| `heap_pop_min` | Root out, last element in, sift down | Swap with the *smaller* child; guard the right-child bound |
| `heap_peek` | Read the min without removing | `false` on empty, `*out` untouched |
| `heap_size` | Element count | — |
| `heap_from_array` | Build from `a[0..n)` bottom-up | Must be O(n) sift-downs from `(n/2)-1` to 0, not n pushes |
| `heap_sort` | Sort ascending via the heap | Heapify then repeatedly pop min (or equivalent) |
| `heap_validate` | Checks every element ≤ its children and `len <= cap` | Parent/child is the *only* ordering a heap promises |

Related: `shortestpath`'s Dijkstra is the canonical priority-queue consumer;
`sorting`'s `heap_sort_arr` re-implements heapsort without the struct.

## trie

**Purpose** — A prefix tree over `'a'..'z'`: insert, whole-word lookup,
prefix lookup, and remove with pruning of dead branches.

**Background** — A trie stores strings by *path*, not by node: each edge is
a character, and a word is a root-to-node path with the endpoint flagged
`is_word`. That flag is the whole semantic content of the structure — it is
what distinguishes an inserted *word* from a mere *prefix* (after inserting
"apple", `contains("app")` is false but `starts_with("app")` is true), and
forgetting to check it is the number-one trie bug. Lookup is O(length of
key) regardless of how many keys are stored — no hashing, no comparisons
against other keys — which is why tries power autocomplete, spell-checkers,
and longest-prefix-match in IP routing tables. The memory story is the
trade-off: 26 child pointers per node (208 bytes on 64-bit) is enormously
wasteful for sparse data, which is why production systems use compressed
variants — radix/Patricia tries (collapse single-child chains; used in
Linux's routing tables) and adaptive radix trees (ART, used in in-memory
databases like DuckDB) that size nodes to their fan-out. Deletion is the
subtle operation: unmark `is_word`, then free nodes bottom-up — but *only*
nodes with no children and no `is_word` flag, or you destroy other words
sharing the prefix. The empty string is a valid word here (it terminates at
the root), a boundary case the tests exercise.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `trie_init` | Allocate the root (always non-NULL after init) | Root exists even for an empty trie — "" terminates there |
| `trie_free` | Free every node | Postorder walk; safe to call twice |
| `trie_insert` | Insert a word, creating nodes as needed | Returns `false` if already present — check the final `is_word` |
| `trie_contains` | Whole-word membership | Path must exist *and* end on `is_word == true` |
| `trie_starts_with` | Prefix membership | Path existence alone suffices; every word starts with "" |
| `trie_remove` | Unmark, then prune dead nodes bottom-up | Prune only childless, non-word nodes — shared prefixes must survive |

Related: `hashchain`/`hashopen` answer "exact key lookup" with different
trade-offs; the trie is the only one that answers prefix queries.

## unionfind

**Purpose** — Disjoint-set union over `0..n-1` with union by rank and path
compression, tracking the live component count.

**Background** — Union-find answers one question — "are these two elements
in the same group?" — under a stream of merges, and it answers it in
effectively constant time: with both optimizations, m operations cost
O(m·α(n)) where α is the inverse Ackermann function, ≤ 4 for any n that fits
in this universe. The two optimizations are independent and both easy to get
subtly wrong. Union by *rank* attaches the shorter tree under the taller
one; rank is an upper bound on height, and it only increments when two
equal-rank roots merge — incrementing it on every union quietly destroys
the bound. Path compression makes `find` point every visited node directly
at the root; the two-pass version (find root, then rewrite) and the
recursive one-liner (`parent[x] = find(parent[x])`) are both fine, but note
compression mutates state, which is why `uf_find` takes a non-const pointer
— a nice C lesson in how const-correctness documents side effects. The
`count` field starts at n and drops only on *successful* unions (merging
two distinct sets), giving O(1) connected-components queries. Memory-wise
this is two flat int arrays — as cache-friendly as algorithms get.
Union-find shows up in interviews via Kruskal, cycle detection in undirected
graphs, and "number of islands" variants.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `uf_init` | n singleton sets: `parent[i] = i`, rank 0 | `count` starts at n; n == 0 is legal |
| `uf_free` | Free both arrays | Safe to call twice |
| `uf_find` | Root of x's set, compressing the path | Mutates `parent[]` — that's the point |
| `uf_union` | Merge by rank; `true` iff two sets became one | Rank bumps *only* on equal-rank merges; decrement `count` only on success |
| `uf_connected` | Same-set test | Just `find(a) == find(b)` |
| `uf_count` | Live component count | O(1) because unions maintain it |

Related: `mst`'s Kruskal is the canonical application (it embeds its own
private DSU); `graph` cycle detection in undirected graphs is another.

## graph

**Purpose** — A directed graph as adjacency lists, with BFS, recursive and
iterative DFS (identical visit order), Kahn's topological sort, and cycle
detection.

**Background** — Adjacency lists (here, one growable int array per vertex)
are the default graph representation because real graphs are sparse: O(n +
m) space versus a matrix's O(n²), and iterating a vertex's neighbors streams
through a contiguous array. This module pins down something most courses
leave vague: **visit order is part of the contract**. Neighbors are visited
in edge-insertion order, ties in Kahn's algorithm break toward the smallest
vertex — so outputs are deterministic and the differential tests can compare
exact sequences, not just sets. The star exercise is `dfs_order_iter`:
making an explicit-stack DFS produce *exactly* the recursive preorder. The
naive translation (push neighbors in order) visits them backwards, because a
stack reverses; the fix is pushing neighbors in *reverse* adjacency order,
and deferring the visited-check to pop time (or you'll visit a vertex twice
when it's pushed twice before being popped). This recursive→iterative
transformation is a staple interview question precisely because the naive
version is wrong in a way that looks right. Kahn's algorithm doubles as
cycle detection for free: if the indegree-0 queue drains before all n
vertices are output, whatever remains lies on or downstream of a cycle.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `graph_init` / `graph_free` | n vertices, no edges / free all lists | Parallel edges and self-loops are allowed |
| `graph_add_edge` | Append directed edge u→v to u's list | Append order *is* the visitation contract |
| `graph_bfs_order` | Queue BFS from src; fills `out[]`, returns count reached | Mark vertices when *enqueued*, not dequeued, or they enter twice |
| `graph_dfs_order` | Recursive preorder DFS | Visit, then recurse into unvisited neighbors in order |
| `graph_dfs_order_iter` | Explicit-stack DFS, same order as recursive | Push neighbors in *reverse*; skip already-visited on pop |
| `graph_topo_sort` | Kahn's algorithm, smallest ready vertex first | `false` on cycle: fewer than n vertices ever reach indegree 0 |
| `graph_has_cycle` | Directed-cycle test | Kahn's leftover count, or DFS with a gray/in-progress state |

Related: `shortestpath` and `mst` build on the same adjacency-list idea with
weights; `stackqueue` supplies the queue BFS needs.

## shortestpath

**Purpose** — Single-source shortest paths on a weighted digraph: Dijkstra
(non-negative weights) and Bellman-Ford (negative weights, with reachable
negative-cycle detection), cross-checked against each other.

**Background** — The two algorithms embody a trade-off worth being able to
articulate: Dijkstra is greedy — it permanently settles the closest
unsettled vertex, which is only valid because non-negative weights guarantee
no later path can undercut a settled one; a single negative edge breaks that
proof (a precondition the header pointedly does not check). Bellman-Ford
makes no such assumption: relax every edge n-1 times, because the longest
possible simple path has n-1 edges; if an n-th pass still improves anything,
a negative cycle is reachable — that detection is *the* reason the algorithm
survives (e.g. currency-arbitrage detection, and distance-vector routing
protocols like RIP are distributed Bellman-Ford). Complexity: this module's
Dijkstra is the O(n²) scan-for-closest form, which the header explicitly
blesses — and on dense graphs (m ≈ n²) it genuinely beats the O((n+m) log n)
binary-heap version, a concrete case of the "worse big-O" winning. Two
conventions here prevent the classic bugs: distances are `long long` so
sums of int weights can't overflow (an INT_MAX "infinity" that gets added
to is UB Bellman-Ford implementations hit constantly), and unreachable is
-1, distinguished from every real distance.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `wgraph_init` / `wgraph_free` | Weighted digraph over `0..n-1` / free | Same adjacency-list shape as `graph`, entries carry a weight |
| `wgraph_add_edge` | Add directed u→v with weight w | Weights may be negative — only Dijkstra forbids that |
| `dijkstra` | Greedy settle-the-closest; fills `dist[]` | O(n²) scan is fine; never relax from an unreached (-1) vertex |
| `bellman_ford` | n-1 relaxation passes; `false` if a reachable negative cycle exists | Skip relaxing edges out of still-unreachable vertices, or -1 + w poisons `dist` |

Related: `heap` provides the priority queue the faster Dijkstra variant
would use; `graph` is the unweighted foundation; the tests cross-check the
two algorithms as each other's oracle.

## mst

**Purpose** — Minimum spanning tree weight of a connected undirected
weighted graph, computed two independent ways: Kruskal (edge-sorting +
union-find) and Prim (grow-from-a-vertex).

**Background** — Both algorithms are greedy and both are correct for the
same underlying reason, the *cut property*: the cheapest edge crossing any
cut belongs to some MST. Kruskal takes edges globally cheapest-first and
adds each one that doesn't close a cycle — which is exactly the question
union-find answers, making it O(m log m) dominated by the sort. Prim grows
one tree from vertex 0, repeatedly adding the cheapest edge to a new vertex;
the simple O(n²) form the header endorses is, as with dense-graph Dijkstra,
genuinely the right choice when m ≈ n² — the second showcase in this repo of
the asymptotically "worse" option winning. The contract has two sharp
edges. First, `kruskal` must build its own private DSU inside `mst.c` (each
module compiles standalone — no `#include "unionfind.h"`), which is a
worthwhile exercise: the whole structure is ~15 lines once you've internalized
it. Second, the input edge array is `const` and must not be modified — sort
a *copy*, an ownership discipline that mirrors real API design. Disconnected
input has no spanning tree: both functions return -1, and the differential
test cross-checks the two algorithms' totals on random graphs (the MSTs may
differ under weight ties; the total weight may not).

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `kruskal` | Sort edges by weight; union-find admits cycle-free edges until n-1 are placed | Sort a *copy* (input is const); embed a private DSU; self-loops can never join components |
| `prim` | Grow the tree from vertex 0, cheapest connecting edge each round | O(n²) form is fine; return -1 if some vertex is never reached |

Related: `unionfind` is the standalone version of Kruskal's engine;
`shortestpath`'s Dijkstra looks nearly identical to Prim but optimizes a
different quantity (path length vs. single edge weight) — confusing the two
is a classic exam trap.

## sorting

**Purpose** — Five classic sorts behind one API shape (`a[0..n)` ascending):
insertion, merge, quick (with sane pivots), heapsort, and a hybrid.

**Background** — The point of implementing five sorts is the trade-off
matrix, because no single sort dominates. Insertion sort is O(n²) but has
tiny constants, is stable, adaptive (O(n) on nearly-sorted input), and works
in cache — which is why it wins below a few dozen elements and why
`hybrid_sort` exists. Merge sort guarantees O(n log n) and stability but
pays O(n) extra memory and a copy-heavy merge; its access pattern is
sequential, which caches like. Quicksort is in-place with the best cache
behavior of the O(n log n) sorts, but naive first-element pivoting hits
its O(n²) worst case on *already-sorted* input — the most common real input
there is — and all-equal arrays can do the same to a careless partition;
median-of-three (or random) pivots are the contract's required defense, and
adversarial inputs are exactly what the tests feed it. Heapsort is the
safety net: O(n log n) worst case, in-place, but with hostile cache behavior
(each sift-down jumps across the array), so it's slower in practice —
introsort, the real-world standard (C++ `std::sort`), runs quicksort and
falls back to heapsort when recursion depth betrays an adversarial input.
The `hybrid_sort` here (quicksort that hands subranges < 16 to insertion
sort) is the other half of that standard design. Every one of these
trade-offs is interview material; "why is your quicksort O(n²) on sorted
input, and what do you do about it?" is a rite of passage.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `insertion_sort` | Stable O(n²); shift-and-insert | Great on tiny/nearly-sorted input — that's why hybrid uses it |
| `merge_sort` | Stable O(n log n) with a temp buffer | Merge must be stable: take from the *left* run on ties |
| `quick_sort` | Expected O(n log n), median-of-three or random pivot | Sorted and all-equal inputs must not recurse to death |
| `heap_sort_arr` | In-place heapsort, no heap struct | Worst-case O(n log n); poor locality is the price |
| `hybrid_sort` | Quicksort, switching to insertion sort below 16 elements | The standard practical optimization (half of introsort) |

Related: `heap` builds the same sift-down machinery as a data structure;
`search` requires the sorted arrays these produce. All five are
differential-tested against `qsort`.

## search

**Purpose** — Binary search over a sorted `int` array, plus `lower_bound` /
`upper_bound` with exact `std::lower_bound`/`std::upper_bound` semantics.

**Background** — Binary search is famously the algorithm everyone knows and
almost nobody writes correctly: Bentley reported that ~90% of professional
programmers failed to produce a correct one, and even Java's
`Arrays.binarySearch` shipped for nine years with an overflow bug —
`(lo + hi) / 2` overflows when the sum exceeds `INT_MAX`; the fix is
`lo + (hi - lo) / 2`. The deeper skill this module teaches is the
**loop-invariant discipline** behind `lower_bound`/`upper_bound`: maintain a
half-open range `[lo, hi)` where everything left of `lo` fails the predicate
and everything from `hi` on satisfies it, and shrink until `lo == hi`. Get
the invariant right and the off-by-ones resolve themselves; get it wrong and
you flail between `hi = mid` and `hi = mid - 1` until the tests pass by
luck — which is precisely why these are fuzzed against a linear scan across
every boundary case (empty array, key below all, above all, long runs of
duplicates). The pair is more useful than plain membership: `[lower_bound,
upper_bound)` is exactly the run of equal keys, so their difference counts
occurrences. Cache-wise, binary search's jumping access pattern is hostile
(each probe is a likely miss), which is why B-tree-style layouts and
branchless/Eytzinger variants exist for hot paths — worth knowing even
though the plain version is what's required here.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `binary_search_idx` | *Some* index holding key, or -1 | Any occurrence is acceptable under duplicates; overflow-safe midpoint |
| `lower_bound` | First index with `a[i] >= key`, else n | Half-open `[lo, hi)` invariant; returns `size_t`, never -1 |
| `upper_bound` | First index with `a[i] > key`, else n | Identical loop, `>` vs `>=` — one character, different function |

Related: `sorting` produces the arrays these functions require; the
lower/upper pair reappears inside `strdp`'s O(n log n) LIS.

## strdp

**Purpose** — String algorithms and canonical dynamic programming: the KMP
prefix function and search, Levenshtein edit distance, longest increasing
subsequence, coin change, and 0/1 knapsack.

**Background** — Two families share this module. The string half is KMP: the
prefix function `pi[i]` records the longest proper prefix of `s[0..i]` that
is also its suffix, and the non-obvious insight is the *fallback chain* — on
a mismatch you retreat to `pi[k-1]`, possibly repeatedly, reusing everything
already matched so the search never re-examines a haystack character. That's
what makes it O(|haystack| + |needle|) versus naive matching's O(n·m); the
same failure-link idea generalizes to Aho-Corasick multi-pattern matching
(grep, intrusion detection). The DP half is four canonical problems chosen
because each teaches a distinct recurrence shape: edit distance is a 2-D
table over prefix pairs (the engine of diff, spell-correction, and
bioinformatics alignment) where the classic bug is mishandling the empty
prefix — row/column 0 must be i and j, not 0; LIS is DP over "best ending
here" with an O(n²) form and an O(n log n) refinement built on `lower_bound`
(the tails array — note *strictly* increasing is required, which decides
lower vs. upper bound); coin change is an unbounded 1-D knapsack where an
"impossible" sentinel must not be treated as a real minimum (initialize with
a large value or -1, not something you'd accidentally take `min` of); and
0/1 knapsack's one-copy-per-item constraint forces the weight loop to run
*backwards* in the 1-D formulation — run it forward and each item can be
taken twice, silently turning it into the unbounded variant. Every one of
these is a top-tier interview staple; the edge cases in the header (empty
needle matches at 0, `amount == 0 -> 0`) are exactly the ones interviewers
poke at.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `prefix_function` | KMP failure array: `pi[i]` = longest proper prefix of `s[0..i]` that's also a suffix | *Proper* prefix, so `pi[0] == 0` always; fall back through `pi[k-1]`, not to 0 |
| `kmp_search` | First occurrence of needle in haystack, or -1 | Empty needle matches at 0; must be O(n + m) — that's the point of KMP |
| `edit_distance` | Levenshtein distance between two strings | Base row/column are i and j (cost of building from empty), not 0 |
| `lis_length` | Longest *strictly* increasing subsequence length | Strictness decides lower_bound vs upper_bound in the O(n log n) form |
| `coin_change_min` | Fewest coins summing exactly to amount, or -1 | Don't let the "impossible" sentinel win a `min`; `amount == 0 -> 0` |
| `knapsack_01` | Max value within weight capacity, each item at most once | 1-D DP must iterate weights *backwards* or items get reused |

Related: `search`'s `lower_bound` powers the fast LIS; KMP's failure links
are conceptually cousins of `trie` (Aho-Corasick is a trie with failure
links).
