# ProperNet — module documentation

Per-function documentation and conceptual background for every module. The
headers in `src/` remain the authoritative contracts; this file adds the
protocol and API background — why network byte order exists, what the URL and
HTTP grammars actually are, why partial `send`/`recv` is the normal case, and
where poll-based multiplexing came from. Modules are listed in the Makefile's
(recommended) order.

## byteorder

**Purpose** — Reimplement the classic `htons`/`ntohs`/`htonl`/`ntohl`
quartet portably — no endianness `#ifdef`s, correct on any host.

**Background** — Different CPUs disagree about which end of a multi-byte
integer comes first in memory: x86 and ARM (as commonly configured) are
little-endian, network protocols standardized on big-endian ("network byte
order") back in the era of big-endian workstations — RFC 1700 canonized it.
So every port number and address that crosses the wire must be converted at
the boundary, which is what the `hton*`/`ntoh*` family does. The deeper
lesson of this module is that you never need to *detect* the host's
endianness: shifting a value right by 8, 16, 24 bits gives you its bytes in a
layout-independent way, and writing those bytes in wire order (most
significant first) then `memcpy`ing them back into an integer is correct
everywhere. Code with `#if BYTE_ORDER == ...` is fragile; code built on
shifts is portable by construction. The tests check both agreement with the
system functions on many values *and* the actual byte layout in memory, so an
implementation that swaps unconditionally (or never) fails.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `my_htons` | Host-order 16-bit value → network (big-endian) order | Build the wire bytes with shifts; never test the host's endianness |
| `my_ntohs` | Network-order 16-bit value → host order | Read the bytes back out and reassemble with shifts |
| `my_htonl` | Host-order 32-bit value → network order | Same construction, four bytes: MSB first on the wire |
| `my_ntohl` | Network-order 32-bit value → host order | Unconditional byte-swapping is just as wrong as a no-op — both fail on half the world's machines |

## urlparse

**Purpose** — Parse `http`/`https` URLs of the shape
`scheme://host[:port][/path...]` into a fixed-size `UrlParts` struct,
rejecting malformed input.

**Background** — The generic URL grammar (RFC 3986) is
`scheme://authority/path?query#fragment`; this module implements the
practical HTTP subset a client needs before it can call `connect`: scheme
determines the default port (80 for http, 443 for https — the reason you
never type `:443` after an `https://` URL), host is everything between `://`
and the first `:` or `/`, and the path defaults to `/` because an HTTP
request line must always carry a target. Parsing into *fixed-size* buffers is
deliberate C discipline: every component length is checked against its array
before copying, so over-long input is rejected rather than truncated or
overflowed — the exact place real-world parsers have historically had their
buffer-overflow CVEs. The port rules encode the classic validation trio:
non-empty, all digits, and range-checked against 65535 (a port is a 16-bit
field in the TCP header, which is *why* the limit is what it is). Query
strings and fragments are intentionally left inside the path — splitting them
is a later refinement, not part of this contract.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `url_parse` | Parses `url` into `UrlParts` (scheme, host, port, path); `true` on success, `false` on any malformed input | Only `http`/`https` accepted; empty host rejected; missing port defaults by scheme (80/443); missing path becomes `"/"`; any over-long component is rejected, not truncated |

## httparse

**Purpose** — Parse a raw HTTP/1.1 request head (request line + headers) out
of a length-bounded, not-NUL-terminated byte buffer, plus case-insensitive
header lookup.

**Background** — RFC 7230 defines the request head:
`METHOD SP TARGET SP VERSION CRLF`, then header lines of `name ":" OWS
value CRLF`, terminated by an empty line — i.e. `CRLF CRLF` ends the head and
everything after it is body. The central discipline here is that network data
is *not a C string*: `buf` may lack a NUL, may contain garbage past `len`,
and may even contain embedded NULs (which this parser rejects inside the
head, since a NUL in a header is at best malformed and at worst a smuggling
vector). Every scan must be bounded by `len` — `strstr`/`strchr` on the raw
buffer are wrong by construction. The grammar rules encode real-server
behavior: exactly three request-line tokens separated by single spaces,
non-empty header names before the `:`, optional whitespace after it trimmed
from the value, and a hard cap on header count (real servers cap everything,
because unbounded input is a denial-of-service invitation). Header names are
case-insensitive per the RFC — `Host`, `host` and `HOST` are one header —
which is why lookup is a separate function rather than `strcmp` at the call
site.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `http_parse_request` | Parses the request head from the first `len` bytes of `buf` into an `HttpRequest`; `false` on truncated/malformed input | `buf` is not NUL-terminated — every read must be bounded by `len`; the head must end in `\r\n\r\n` within `len` or it's rejected as incomplete |
| `http_get_header` | Returns the value of the first header matching `name` case-insensitively, or `NULL` | Case-insensitive comparison is the RFC rule, not a convenience |

## sockets

**Purpose** — The TCP building blocks under every client and server:
loop-until-done send, delimiter-bounded receive, and loopback
listen/connect helpers with ephemeral-port support.

**Background** — TCP is a *byte stream*: it preserves byte order but not
message boundaries, so one `send` may arrive as three `recv`s and vice versa
— nothing about your `write` sizes survives the trip. That makes two loops
mandatory in all real code: `send_all`, because `write` on a socket may
accept only part of your buffer when the kernel's send buffer is full (and
may be interrupted by a signal, hence the `EINTR` retry); and `recv_until`,
because "read one line" means accumulating bytes until the delimiter shows
up, however many syscalls that takes. The tests enforce this honestly — a
client thread dribbles bytes one at a time over a real loopback connection,
so any single-shot `read`/`write` implementation fails. On the setup side:
`SO_REUSEADDR` lets a restarted server rebind while the old socket lingers in
TIME_WAIT; binding port 0 asks the kernel for an ephemeral port, and
`getsockname` reveals which one you got (how the tests avoid port
collisions); and every error path must close what it opened — fd leaks in
listen/connect helpers are bugs the tests' fd hygiene will expose.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `send_all` | Writes all `n` bytes to `fd`, looping over partial writes and retrying on `EINTR`; returns `n` or -1 | Partial writes are normal socket behavior, not an error — resume where you left off |
| `recv_until` | Reads into `buf` until the delimiter is stored (included), `cap` is reached, or EOF/error; returns bytes stored | Result is NOT NUL-terminated; 0 means EOF before any byte; byte-by-byte reading is legitimately correct here |
| `tcp_listen_loopback` | Creates a listener bound to 127.0.0.1, `SO_REUSEADDR`, backlog ≥ 8; writes the actual port back through `port_inout` | Port 0 = kernel-picked ephemeral port, recovered with `getsockname`; no fd leaks on error paths |
| `tcp_connect_loopback` | Connects to 127.0.0.1:`port`; returns the connected fd or -1 | Same fd hygiene: close the socket on a failed `connect` |

## mux

**Purpose** — A single-threaded, `poll(2)`-based multi-client echo server:
one thread serving interleaving concurrent clients without blocking on any
one of them.

**Background** — The alternative designs frame why this exists: a
blocking server can only talk to one client at a time, and
thread-per-connection stops scaling long before the connection counts modern
servers need — the "C10K problem" (Kegel, 1999) that pushed the field toward
event-driven I/O. The readiness model inverts control: instead of blocking in
`read` on one fd, you hand the kernel your whole fd set and block in *one*
place — `poll` — which returns when any fd is ready. The loop shape is
universal: poll → check the listener (`accept` new clients into the set) →
check each client (`POLLIN` → `read` a chunk, echo it back with a full write;
`read` == 0 → EOF, close and remove from the set). `poll` is used here rather
than `epoll`/`kqueue` deliberately — those are OS-specific and earn their
complexity at scale (that's the later Phase 4 HTTP-server project on Linux);
`poll` is portable POSIX and teaches the identical loop. The tests run three
interleaved clients that each verify they get back exactly their own bytes
in order, so per-client state (whose fd is whose) can't be fudged. Even
inside an event loop, echo writes must handle partial writes — readiness for
reading says nothing about how much the send buffer will take.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `echo_poll_serve` | Serves echo to `nclients_expected` clients on `listen_fd` with a single `poll` loop; returns 0 once all have connected and disconnected, -1 on bad args or fatal error | Stop polling the listener once all expected clients were accepted; `read` returning 0 is a client hanging up (close its fd, not an error); never block on any single client |

**Related:** the loop shape here is the poll-based cousin of the ProperOS
`atomics` SPSC ring (events queued by one side, drained by the other) — and
`send_all`-style partial-write handling from `sockets` is a hard prerequisite.
