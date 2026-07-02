# ProperC — module documentation

Per-module background and per-function reference for the ProperC self-test.
Each module verifies one cluster of core-C skills; the doc comments in
`src/<module>.h` remain the authoritative contract for every function — this
file adds the *why*: what's happening at the machine level, the classic
pitfalls each exercise is built around, and how the modules relate.

## pointers

**Purpose** — Verifies that you can move through memory with raw pointers:
arithmetic, in-place array manipulation, and byte-level copying with and
without overlap.

**Background** — A pointer is just an address plus a type, and the type is
what gives `p + 1` its meaning: it advances by `sizeof(*p)` bytes, not by one
byte. Arrays decay to pointers at almost every use, so `arr[i]` is literally
`*(arr + i)` — understanding that equivalence is the foundation for everything
else in C. The `memcpy` vs `memmove` split is the classic overlap question:
`memcpy` may copy in any order (and real implementations copy in wide chunks),
so if the regions overlap it can read bytes it has already clobbered;
`memmove` must behave *as if* the source were first copied to a temporary
buffer, which in practice means choosing the copy direction based on how the
regions overlap. This exact distinction — and the in-place rotate, which is
usually solved with the three-reversal trick — are perennial interview
questions precisely because they force you to reason about which byte is
written when.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `void swap_int(int *a, int *b)` | Swap the two ints pointed to | Dereference to touch the values, not the pointers |
| `void reverse_array(int *arr, size_t n)` | Reverse `arr[0..n)` in place | Two-index walk; `n == 0` must be a no-op (careful with unsigned `n - 1`) |
| `void *my_memcpy(void *dst, const void *src, size_t n)` | Copy `n` bytes, regions must not overlap | Copy through `unsigned char *`; returns `dst` |
| `void *my_memmove(void *dst, const void *src, size_t n)` | Copy `n` bytes, regions may overlap | Pick copy direction (forward/backward) so nothing is read after being overwritten |
| `void rotate_left(int *arr, size_t n, size_t k)` | Rotate left by `k` in place | Reduce `k % n` first (and don't divide by `n == 0`); reverse-reverse-reverse trick |

Related: `strings` (pointer traversal with a sentinel instead of a length) /
study guide §1.2 pointers & arrays.

## strings

**Purpose** — Verifies C-string fundamentals: NUL-terminated traversal,
copying with and without bounds, lexicographic comparison, and substring
search.

**Background** — A C string is not a type; it's a convention — a `char` array
where a `'\0'` byte marks the end. Every function in this module is really an
exercise in respecting that sentinel: forget to copy it and the consumer runs
off the end of the buffer; forget it's *not* counted by `strlen` and your
allocation is one byte short (the classic off-by-one). `strncpy` is here
because its semantics surprise almost everyone: it zero-fills the remainder
when the source is short, and — the sharp edge — it does **not** NUL-terminate
when the source is at least `n` long, which is how "safe" code ends up with
unterminated strings. `strcmp` returns the sign of the first differing byte
*compared as `unsigned char`*, a detail that matters once bytes ≥ 0x80 appear
(on platforms where plain `char` is signed, a naive `*a - *b` gets the sign
wrong). These functions are the standard warm-up in systems interviews:
implementing `strstr` or reversing a string in place shows whether you can
write a bounds-correct loop without a library.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `size_t my_strlen(const char *s)` | Length excluding the terminator | The `'\0'` is not counted but is always there |
| `char *my_strcpy(char *dst, const char *src)` | Copy including the NUL | Must copy the terminator; caller guarantees room |
| `char *my_strncpy(char *dst, const char *src, size_t n)` | strncpy semantics | Zero-fills short sources; does NOT terminate long ones |
| `int my_strcmp(const char *a, const char *b)` | negative/zero/positive comparison | Compare as `unsigned char`, not plain `char` |
| `char *my_strcat(char *dst, const char *src)` | Append in place | Walk to the end of `dst` first, then copy with NUL |
| `char *my_strchr(const char *s, int c)` | First occurrence of `(char)c`, or NULL | The terminator counts: `strchr(s, 0)` finds it |
| `char *my_strstr(const char *haystack, const char *needle)` | First substring occurrence, or NULL | Empty needle returns `haystack` |
| `void str_reverse(char *s)` | Reverse in place | Don't move the terminator; two-pointer swap |
| `bool is_palindrome(const char *s)` | Same forwards and backwards | Empty string is true |

Related: `safety` (the bounded, overflow-proof versions of copy/concat) /
study guide §1.2 strings.

## memory

**Purpose** — Verifies heap discipline: `malloc`/`realloc`/`free`, ownership
transfer, and amortized growth via a small growable vector.

**Background** — Manual memory management is the defining responsibility of C.
Every allocation creates an ownership question — who frees this, and exactly
once? — and the bugs from getting it wrong (leaks, double frees, use after
free) are exactly what AddressSanitizer exists to catch. `my_strdup` is the
minimal ownership-transfer exercise: allocate `strlen + 1` bytes (the +1 is
the most common mistake) and hand the caller the obligation to `free`. The
`IntVec` is the canonical dynamic array: growth by doubling gives amortized
O(1) push, and the key implementation detail is using `realloc`'s return value
correctly — assigning it straight to `v->data` loses the original pointer
(and leaks it) if `realloc` fails, which is why the contract requires the
vector to be *unchanged* on failure. `vec_free` resetting to the empty state
makes double-free safe by construction, a pattern worth internalizing for any
resource-owning struct.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `char *my_strdup(const char *s)` | malloc'd copy, caller frees | Allocate `strlen(s) + 1`; NULL only on allocation failure |
| `void vec_init(IntVec *v)` | Empty vector, no allocation | `data` may be NULL; `len == cap == 0` |
| `bool vec_push(IntVec *v, int x)` | Append, growing as needed | Stash `realloc`'s result in a temp — on failure the vector must be unchanged |
| `bool vec_pop(IntVec *v, int *out)` | Remove last into `*out` | Returns false on empty and leaves `*out` untouched |
| `int vec_get(const IntVec *v, size_t i)` | Element `i` | Precondition `i < len` — no bounds check required |
| `void vec_free(IntVec *v)` | Free and reset to empty | Safe to call twice because it resets state |

Related: `linkedlist` (per-node allocation instead of one growable block) /
study guide §1.2 dynamic memory.

## bits

**Purpose** — Verifies fluency with bitwise operators: masking, setting/
clearing/toggling single bits, popcount, and bit-order manipulation.

**Background** — Bit manipulation is how C talks to hardware registers, flag
fields, and packed formats, and it's a fixture of systems interviews because a
few idioms unlock everything: `x & (x - 1)` clears the lowest set bit (the
heart of both popcount and the power-of-two test), `1u << n` builds a
single-bit mask, and `x & -x` isolates the lowest set bit. Two pitfalls the
sanitizers will hold you to: shift *amounts* must be less than the bit width
(shifting a 32-bit value by 32 is undefined behavior — relevant to
`count_trailing_zeros(0)`, which is *defined* here as 32, so you can't compute
it with one naive shift-based loop bound), and shifting `1` (a signed int)
left by 31 is UB while `1u << 31` is fine — always build masks from unsigned
literals. Working on `uint32_t` keeps everything width-exact so results don't
vary by platform.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `int count_set_bits(uint32_t x)` | Popcount | `x &= x - 1` loop runs once per set bit |
| `bool is_power_of_two(uint32_t x)` | Exactly one bit set | 0 must return false: `x && !(x & (x - 1))` |
| `uint32_t reverse_bits(uint32_t x)` | Mirror all 32 bits | Bit 0 ↔ bit 31; loop or mask-and-shuffle |
| `uint32_t set_bit / clear_bit / toggle_bit(uint32_t x, int n)` | Single-bit edits | Build the mask as `1u << n` (unsigned!), clear with `& ~mask` |
| `int count_trailing_zeros(uint32_t x)` | Zeros from the LSB | Defined as 32 for `x == 0` — don't shift by 32 to get there |

Related: `structs` and `ub` (shifts and masks applied to serialization and
safe arithmetic) / study guide §1.2 bitwise operations.

## structs

**Purpose** — Verifies that you understand data representation: byte order,
serializing multi-byte values to defined layouts, and packing fields into a
word.

**Background** — A `uint32_t` in memory is four bytes, and *which byte comes
first* is endianness: little-endian machines (x86, Apple Silicon) store the
least-significant byte at the lowest address. The moment data crosses a
machine boundary — network protocols, file formats — you must serialize to an
*explicit* byte order rather than whatever the host happens to use. The
header's instruction to avoid `*(uint32_t *)bytes` is the load-bearing lesson:
that cast risks both misaligned access and strict-aliasing undefined behavior,
whereas shifts and masks (`v >> 24`, `v & 0xFF`) express byte extraction in
pure arithmetic that works identically on any host — the compiler turns it
into a single load/byte-swap anyway. Detecting endianness itself requires
inspecting bytes through `unsigned char *` (the one aliasing-legal window into
any object). `pack_rgba` is the same skill in miniature: a defined
most-significant-first layout (`0xRRGGBBAA`) built with shifts, regardless of
host order.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `bool is_little_endian(void)` | True iff LSB is stored first | Inspect a `uint32_t`'s first byte via `unsigned char *` or `memcpy` |
| `void u32_to_bytes_le(uint32_t v, uint8_t out[4])` / `uint32_t u32_from_bytes_le(const uint8_t in[4])` | (De)serialize little-endian | Shifts + masks, never a pointer cast; works on any host |
| `void u32_to_bytes_be(...)` / `uint32_t u32_from_bytes_be(...)` | (De)serialize big-endian | Same code, opposite byte indices — network byte order is BE |
| `uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)` | Pack channels as `0xRRGGBBAA` | `r` occupies the most-significant byte |
| `void unpack_rgba(uint32_t rgba, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a)` | Inverse of pack | Each out pointer may be NULL — check before writing |

Related: `ub` (`load_u32_le` is the same idea under a misalignment constraint)
/ ProperMachine `layout` / study guide §1.3 data representation.

## funcptr

**Purpose** — Verifies function pointers as values: writing generic
higher-order operations (map/filter/reduce) and a comparator-driven sort.

**Background** — A function pointer is just the address of code, and passing
one around is how C does callbacks, dispatch tables, and pluggable policy —
it's the mechanism behind `qsort`, signal handlers, VFS operation tables in
the kernel, and (via structs of function pointers) hand-rolled vtables. The
syntax is the first hurdle (`int (*fn)(int)` — the parentheses matter, or
you've declared a function returning a pointer), but the deeper skill is
respecting the *contract* of a callback: `int_reduce`'s fold order
(`acc = op(acc, arr[i])`, left to right) is observable when `op` isn't
commutative, and a comparator's negative/zero/positive convention is exactly
`strcmp`'s — a sort must invoke it consistently and never assume the values
are e.g. only ±1. Filter's "preserve order, return the count written" shape is
the standard compact-into-buffer idiom you'll see all over real C APIs.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `void int_map(int *arr, size_t n, int (*fn)(int))` | Apply `fn` to each element in place | Declarator syntax: `int (*fn)(int)` |
| `size_t int_filter(const int *src, size_t n, int *dst, bool (*pred)(int))` | Copy elements passing `pred`, in order | Returns count written; write index ≠ read index |
| `int int_reduce(const int *arr, size_t n, int init, int (*op)(int, int))` | Left fold from `init` | Order matters for non-commutative `op` |
| `void sort_ints(int *arr, size_t n, int (*cmp)(int, int))` | Ascending sort per comparator | Use only the sign of `cmp`'s result, like `qsort` |

Related: `linkedlist` (another place comparator-style thinking appears) /
study guide §1.2 function pointers.

## linkedlist

**Purpose** — Verifies pointer-linked data structures: building and freeing
node chains, in-place reversal, and the slow/fast (Floyd) pointer techniques.

**Background** — Linked lists matter less as containers (the `bench` drill in
ProperMachine shows why arrays usually beat them cold) and more as the purest
exercise in pointer surgery: every operation is a careful re-aiming of `next`
fields where one wrong ordering loses the rest of the list. In-place reversal
is the canonical three-pointer dance (`prev`/`cur`/`next` — save `next`
*before* overwriting). The slow/fast technique — advance one pointer one node
per step and another two nodes — finds the middle in one pass and, as Floyd's
tortoise-and-hare, detects cycles in O(1) space: if there's a loop, the fast
pointer must eventually land on the slow one. `list_free` has its own classic
bug: you must save `next` before freeing the current node, or you're reading
freed memory (ASan will catch exactly this). These are among the most common
interview warm-ups in existence, and the head-insertion pattern
(`list_push_front` returning the new head) is the standard C idiom for lists
without a wrapper struct.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `Node *list_push_front(Node *head, int val)` | Allocate + prepend; returns new head | On allocation failure returns the *original* head unchanged |
| `size_t list_length(const Node *head)` | Node count (acyclic) | Plain walk to NULL |
| `Node *list_reverse(Node *head)` | Reverse in place; returns new head | Save `next` before rewiring — the three-pointer dance |
| `int list_middle(const Node *head)` | Middle value via slow/fast | Even length ⇒ *second* middle; head is non-NULL by contract |
| `bool list_has_cycle(const Node *head)` | Floyd's tortoise & hare | Fast moves 2, slow moves 1; check `fast && fast->next` before advancing |
| `void list_free(Node *head)` | Free every node | Save `next` before `free(cur)` — never touch freed memory |

Related: `memory` (ownership of each node) / study guide §1.2 pointers &
dynamic data structures.

## safety

**Purpose** — Verifies that you can write string/buffer operations that are
*provably* incapable of writing past the destination — the anti-buffer-
overflow module.

**Background** — Buffer overflows have been the dominant class of C security
vulnerability for four decades: `strcpy` into a fixed buffer with attacker-
controlled input is how stacks get smashed and return addresses overwritten.
The fix is an API shape, not just care: every write takes the destination's
*total capacity* and is contractually bounded by it. This module implements
the OpenBSD `strlcpy`/`strlcat` design, whose two good ideas are (1) the
destination is always NUL-terminated whenever there's any room at all
(`dstsize > 0`), unlike `strncpy`, and (2) the return value is the length the
function *tried* to create, so `ret >= dstsize` is a one-line truncation
check. The sharp edge in `safe_strcat` is the return contract
`min(dstsize, strlen(dst)) + strlen(src)` — it handles the pathological case
where `dst` isn't terminated within `dstsize`. Since the tests run under
AddressSanitizer, even a single byte written past the buffer aborts the run:
the bound isn't a suggestion.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `size_t safe_strcpy(char *dst, size_t dstsize, const char *src)` | Bounded copy, strlcpy-style | Always terminates when `dstsize > 0`; writes nothing when `dstsize == 0`; returns `strlen(src)` |
| `size_t safe_strcat(char *dst, size_t dstsize, const char *src)` | Bounded append, strlcat-style | `dstsize` bounds the *total* buffer, not the appended part; note the `min()` in the return contract |
| `bool copy_n_safe(uint8_t *dst, size_t dstcap, const uint8_t *src, size_t n)` | Copy only if `n <= dstcap` | All-or-nothing: false means not a single byte was written |

Related: `strings` (the unbounded versions these replace) / study guide §1.2
memory safety.

## ub

**Purpose** — Verifies that you can compute correct answers *without ever
executing* the undefined behavior involved: signed overflow, oversized shifts,
and misaligned loads.

**Background** — Undefined behavior isn't "you get a weird value" — it's a
license for the compiler to assume the situation never happens and optimize
accordingly, which is how overflow checks written *after* the overflow
(`if (a + b < a)` for signed ints) get deleted entirely. So every guard here
must run *before* the dangerous operation: `checked_add_i32` compares against
`INT32_MAX - b` / `INT32_MIN - b` (in ranges where those are themselves safe)
rather than adding and looking at the result. The other patterns: shifting a
32-bit value by ≥ 32 is UB even for unsigned types, so `safe_lshift` needs an
explicit `k >= 32` branch; and dereferencing a `uint32_t *` that isn't
4-aligned (or that aliases a buffer of another type) is UB, so `load_u32_le`
must assemble the value from four byte reads and shifts — the portable
deserialization idiom you already met in `structs`. The tests run under UBSan,
which aborts the moment the "forbidden" operation executes, even if the final
answer would have looked right. `avg_no_overflow`'s point is that the obvious
`(a + b) / 2` overflows for large same-sign inputs; a midpoint must be
computed from the difference or via divided halves plus a correction, with the
truncation-toward-zero requirement making the correction term the interesting
part.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `bool checked_add_i32(int32_t a, int32_t b, int32_t *out)` | Add iff representable | Test *before* adding; `*out` untouched on failure |
| `bool checked_mul_i32(int32_t a, int32_t b, int32_t *out)` | Multiply iff representable | Division-based pre-checks; mind `INT32_MIN` and `-1` |
| `int32_t avg_no_overflow(int32_t a, int32_t b)` | Midpoint truncated toward zero | `(a + b) / 2` is the bug being avoided; `avg(INT32_MAX, INT32_MAX) == INT32_MAX` |
| `uint32_t safe_lshift(uint32_t x, unsigned k)` | Shift defined for all `k` | `k >= 32` must yield 0 via a branch, not a shift |
| `uint32_t load_u32_le(const uint8_t *p)` | Little-endian load from unaligned bytes | Byte reads + shifts — never `*(uint32_t *)p` |

Related: `bits` (shift rules) / `structs` (serialization without pointer
casts) / ProperMachine `promotions` / study guide §1.2 undefined behavior.
