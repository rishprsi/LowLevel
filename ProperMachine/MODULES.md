# ProperMachine — module documentation

Per-module background and reference for the ProperMachine self-test: what
each module verifies about how your data actually looks to the machine, and
why it matters. The comments in `src/<module>.h` (and next to each prediction
in `src/<module>.c`) remain the authoritative contract — this file adds the
conceptual background behind the predictions and functions.

## promotions

**Purpose** — Verifies (predict-then-verify, 14 predictions) that you know the
integer promotion rules, the usual arithmetic conversions, unsigned
wraparound, and truncation — before the compiler applies them for you.

**Background** — C almost never does arithmetic in the type you wrote.
Anything smaller than `int` (`char`, `short`, character literals) is promoted
to `int` before any operation, and when a signed and an unsigned operand of
the same rank meet, the *signed one converts to unsigned* — so `-1 < 1U` is
false, because `-1` becomes `UINT_MAX` first. That single rule is behind an
entire genre of real bugs, the worst of which involve `sizeof`: it yields a
`size_t` (unsigned), so `sizeof(int) - 5 > 0` is true even though 4 − 5 is
"negative" — the classic way a loop bound check silently inverts. Unsigned
arithmetic is defined to wrap modulo 2ⁿ (never UB), signed division since C99
truncates toward zero (which forces `-7 / 2 == -3` and pins `-7 % 2` via the
identity `(a/b)*b + a%b == a`), and float-to-int conversion truncates rather
than rounds. Two more standard surprises round out the set: in C (unlike C++)
a character literal has type `int`, so `sizeof('a')` is `sizeof(int)`; and
`sizeof`'s operand is not evaluated (for non-VLA types), so `sizeof(i++)`
never increments `i`. Interviewers love these because the code compiles
cleanly and looks obvious — the only defense is knowing the rules cold.

**Functions / predictions**

| Prediction group | What it probes | Key gotcha/concept |
| --- | --- | --- |
| Signed/unsigned comparison & mixing (`PRED_NEG1_LT_1U`, `PRED_NEG1_PLUS_1U_IS_ZERO`, `PRED_SIZEOF_MINUS_5_GT_0`) | What happens when `int` meets `unsigned` | The signed operand converts to unsigned; `-1` becomes `UINT_MAX`; `sizeof` is unsigned |
| Unsigned wraparound (`PRED_UNSIGNED_WRAP_IS_UINTMAX`, `PRED_SHL31_AS_LL_IS_2P31`) | Modulo-2ⁿ arithmetic and widening an unsigned value | `0u - 1u` is defined, and `1u << 31` widens to `long long` value-preserving (no sign extension) |
| Integer promotion of small types (`PRED_SIZEOF_CHAR_LITERAL`, `PRED_SIZEOF_SHORT_PLUS_SHORT`, `PRED_A_PLUS_1`, `PRED_DIGIT_7_MINUS_0`) | Everything below `int` is promoted before arithmetic | `'a'` is an `int` in C; `short + short` happens in `int`; `'7' - '0'` is the digit idiom |
| Truncation & narrowing (`PRED_NEG7_DIV_2`, `PRED_NEG7_MOD_2`, `PRED_UCHAR_300`, `PRED_TRUNC_3_99`) | Division/modulo direction and conversions that drop information | Division truncates toward zero; unsigned narrowing is reduction mod 256; float→int truncates |
| `sizeof` non-evaluation (`PRED_SIZEOF_NO_SIDE_EFFECT`) | Whether `sizeof(i++)` bumps `i` | The operand is unevaluated for non-VLA types |

Related: ProperC `ub` (where signed overflow — the *undefined* cousin of these
defined rules — is handled) / study guide §1.3 integer representation.

## floats

**Purpose** — Verifies (predictions + implementations) that you understand
IEEE-754 binary32 at the bit level: what's exactly representable, how NaN and
infinity behave, and how to inspect the bit pattern legally.

**Background** — An IEEE-754 float is `[1 sign][8 exponent][23 mantissa]`
bits, and every famous float surprise falls out of that layout. 0.1 and 0.2
are infinite repeating fractions in binary, so `0.1 + 0.2 != 0.3` — but 0.5
and 0.25 are exact powers of two, so their arithmetic is exact; the question
is never "are floats imprecise" but "is this value representable". A 23-bit
mantissa (plus the implicit leading 1) means every integer up to 2²⁴ =
16,777,216 is exact and 16,777,217 is not — the first integer a `float`
cannot hold. NaN compares unequal to *everything including itself* (which is
the standard `x != x` NaN test and the reason NaN poisons sorts), while
dividing a nonzero double by zero is well-defined in IEEE-754 and yields
infinity. On the implementation side, the load-bearing rule is that you may
not read a `float`'s bits through a `uint32_t *` — that's a strict-aliasing
violation (UB) — so `float_bits`/`bits_float` must use `memcpy`, which
compilers turn into a plain register move anyway. Once you hold the bits, NaN
detection, the sign bit of `-0.0f` (invisible to `<` because `-0.0f == 0.0f`),
and ULP distance (for same-sign finite floats, bit patterns order exactly like
the values) are all straightforward — this bit-punning skill is what real
`isnan`, `nextafter`, and float-comparison libraries are made of.

**Functions / predictions**

| Function / group | What it does | Key gotcha/concept |
| --- | --- | --- |
| Representability predictions (`PRED_POINT1_PLUS_POINT2_EQ_POINT3`, `PRED_HALF_PLUS_QUARTER_EQ_3QUARTERS`, `PRED_FLOAT_16777217_EQ_16777216`) | Which decimal/integer values binary floating point holds exactly | Powers of two are exact; 0.1 is not; 2²⁴+1 doesn't fit in 24 mantissa bits |
| Semantics predictions (`PRED_TRUNC_NEG_1_9`, `PRED_NAN_EQ_NAN`, `PRED_ONE_OVER_ZERO_IS_INF`) | Truncation direction, NaN equality, division by zero | `(int)-1.9` truncates toward zero; `NAN == NAN` is false; `1.0/0.0` is `INFINITY`, not UB |
| `uint32_t float_bits(float f)` / `float bits_float(uint32_t)` | Raw bit pattern ↔ float | MUST use `memcpy`; a pointer cast is strict-aliasing UB |
| `bool my_isnan(float f)` | NaN test from the bits only | Exponent all 1s AND mantissa nonzero (zero mantissa = infinity) |
| `int float_sign_bit(float f)` | Sign bit, 0 or 1 | Must return 1 for `-0.0f`, so `f < 0` can't work |
| `uint32_t ulp_diff(float a, float b)` | Distance in units-in-the-last-place | Contract limited to finite, same-sign floats — there bit order == value order |

Related: `promotions` (conversion rules when floats meet ints) / study guide
§1.3 floating point.

## layout

**Purpose** — Verifies (predict-then-verify, 13 predictions) that you can
compute struct sizes and member offsets from the alignment rules — on arm64 /
AAPCS64, where every type's alignment equals its size.

**Background** — The compiler may not reorder struct members, but it must keep
each one aligned, so it inserts invisible padding: each member goes at the
next offset that's a multiple of its alignment, and the total size is rounded
up to the strictest member alignment so that *arrays* of the struct stay
aligned (that last rule is why `sizeof` includes tail padding, and why the
array-stride prediction equals `sizeof`, not the sum of the members). The
classic blunder is ordering members small–big–small: `char, int, char` costs
12 bytes where `int, char, char` costs 8 — real codebases recover significant
memory just by sorting members largest-first. Two subtleties the predictions
target: a nested struct's alignment is its strictest *member's* alignment
(4 for a `char`+`int` struct), not its size; and a lone `char` after a
`long long` costs 7 tail-padding bytes. This knowledge is directly practical —
cache-line packing, serialization mismatches, `offsetof`-based tricks like
`container_of` in the Linux kernel — and "why is `sizeof` this struct 12?" is
a staple systems interview question. Everything here assumes the stated
arm64 ABI; the *rules* are universal even where the numbers differ.

**Functions / predictions**

| Prediction group | What it probes | Key gotcha/concept |
| --- | --- | --- |
| Basic padding (`PRED_SIZEOF_CHAR_INT`, `PRED_OFFSETOF_CHAR_INT_I`, `PRED_SIZEOF_JUST_CHAR`) | Padding before an `int` after a `char`; the no-padding baseline | Members land at multiples of their alignment |
| Member-ordering waste (`PRED_SIZEOF_CHAR_INT_CHAR` vs `PRED_SIZEOF_INT_CHAR_CHAR`) | The char/int/char blunder and its biggest-first fix | Same members, 12 vs 8 bytes |
| Strict alignment & tail padding (`PRED_SIZEOF_CHAR_DOUBLE`, `PRED_OFFSETOF_CHAR_DOUBLE_D`, `PRED_SIZEOF_LONGLONG_CHAR`) | 8-byte-aligned members inside and at the end | A trailing `char` after a `long long` costs 7 tail bytes |
| Nested structs (`PRED_SIZEOF_NESTED`, `PRED_OFFSETOF_NESTED_INNER`) | Alignment of a struct used as a member | It's the strictest member's alignment, not the struct's size |
| Arrays & stride (`PRED_SIZEOF_CHARS_SHORT`, `PRED_OFFSETOF_CHARS_SHORT_S`, `PRED_ARRAY2_STRIDE_BYTES`) | `char[3]` + `short` packing; element-to-element distance | Array stride is exactly `sizeof(element)` — tail padding included |

Related: ProperC `structs` (byte order — the other half of data
representation) / `bench` (why layout affects speed) / study guide §1.3
struct layout.

## bench

**Purpose** — Not a test module: `bench/bench.c` is fully provided and has no
stubs or predictions. `make bench` builds it with `-O2` and *without*
sanitizers (so the timings mean something) and runs two experiments that make
the memory hierarchy visible.

**Background** — Every loop in the benchmark does identical work per element:
load an integer, add it to a sum. Only the *access pattern* differs, so any
timing difference is purely the memory system. Sequential access streams whole
64-byte cache lines (one miss serves many elements) and is predictable enough
that the hardware prefetcher fetches ahead of the loop; pointer-chasing and
huge strides defeat both, turning nearly every access into a cache miss — and
for the matrix, often a TLB miss too — that stalls the core for the full
main-memory latency. This is the empirical basis for "arrays beat linked
lists" and for loop-ordering rules in numerical code.

**The two experiments**

| Experiment | Setup | What the ratio demonstrates |
| --- | --- | --- |
| 1. Array vs shuffled linked list | Sum 1,000,000 `long`s in a contiguous array, then walk 1,000,000 individually heap-allocated list nodes whose visit order was shuffled (deterministic xorshift), after an untimed warm-up pass | Pointer-chasing to unpredictable addresses defeats the prefetcher; each `->next` hop pays full memory latency, typically an order of magnitude or more per element |
| 2. Row-major vs column-major matrix sum | Sum a 4096×4096 `int` matrix (64 MiB) with `j` innermost (stride 4 bytes), then with `i` innermost (stride 16 KiB) | Column order touches a fresh cache line — and often a fresh TLB page — on nearly every access; same arithmetic, wildly different time |

It prints ns/element and the list/array and col/row ratios; there is no
pass/fail. Re-read the numbers after each phase of the curriculum — they don't
change, but what you see in them does.

Related: `layout` (how struct size interacts with cache lines) / ProperC
`linkedlist` (the data structure being humbled here) / study guide §2.4
memory hierarchy.
