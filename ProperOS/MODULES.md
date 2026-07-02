# ProperOS — module documentation

Per-function documentation and conceptual background for every module. The
headers in `src/` remain the authoritative contracts; this file adds the
"why" — which APUE chapter each module comes from, what the kernel is doing
underneath, and the classic pitfalls the tests are designed to catch.
Modules are listed in the Makefile's (recommended) order.

## fileio

**Purpose** — Robust unbuffered I/O with `read(2)`/`write(2)`: retry loops
that survive partial transfers and `EINTR`, a binary-safe file copy, and
`lseek`-based file sizing.

**Background** — This is APUE ch.3. `read` and `write` are direct syscalls
into the kernel's file layer: each open file descriptor is an index into the
process's fd table, which points at a system-wide open-file entry holding
the current offset. The kernel is allowed to transfer *fewer* bytes than you
asked for — a pipe fills up, a signal arrives mid-syscall (`EINTR`), a socket
buffer drains slowly — so single-shot `read`/`write` calls are latent bugs;
robust code always loops. `read` returning 0 means EOF, not an error, and is
how every "read until done" loop terminates. `lseek` only moves the offset in
the open-file entry — it does no I/O at all — which is why seeking to
`SEEK_END` is a cheap way to learn a file's size, provided you restore the
offset you started with (the fd may be shared with someone who cares).

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `read_all` | Reads exactly `n` bytes into `buf`, looping over partial reads and retrying on `EINTR`; returns bytes read (short only at EOF) or -1 | EOF (`read` → 0) ends the loop early and is *not* an error |
| `write_all` | Writes exactly `n` bytes from `buf`, looping over partial writes and retrying on `EINTR`; returns `n` or -1 | A partial write must resume from where it left off, not restart |
| `copy_file` | Copies `src` to `dst` (created/truncated, mode 0644) via `read_all`/`write_all` over a fixed buffer | Must be binary-safe: embedded NUL bytes copied faithfully (no `str*` functions) |
| `file_size_fd` | Returns the size of the file on `fd` using only `lseek` (no `stat`) | The original file offset must be restored before returning |

## procs

**Purpose** — The fork/exec/wait triad: run a program as a child process and
decode its exit status, then capture a child's stdout through a pipe.

**Background** — APUE ch.8. `fork` clones the calling process; the child gets
a copy-on-write duplicate of the address space and a *copy of the fd table*
(the descriptors themselves refer to the same open-file entries). `execv`
then replaces the child's program image entirely — it never returns on
success, so if it *does* return the child must `_exit(127)` rather than fall
back into the caller's code, or you get two processes running the same stack
frame ("returning twice"). The parent must `waitpid` for the child; the raw
status is an encoded value that must be decoded with `WIFEXITED`/
`WEXITSTATUS` (a signal death looks nothing like exit code 0). An unwaited
dead child stays as a zombie holding a process-table slot. Capturing output
adds the ch.15 plumbing: `pipe` + `dup2` onto `STDOUT_FILENO`, and — the
classic bug — closing the write end in the *parent*, or the parent's read
loop never sees EOF.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `run_command` | `fork` + `execv` + `waitpid`; returns the child's exit status (0..255), or -1 on failure or signal death | Decode with `WIFEXITED`/`WEXITSTATUS`; failed `execv` must `_exit(127)`, never return |
| `spawn_capture` | Like `run_command` but redirects the child's stdout into a pipe with `dup2` and reads it into `out` (NUL-terminated) | Close the pipe's write end in the parent or `read` never returns EOF; -1 unless the child exits 0 |

## pipes

**Purpose** — Build shell-style pipelines (`cmd1 | cmd2`, and a 3-stage
version) by hand: pipe plumbing, fd hygiene, and reaping every child.

**Background** — APUE ch.15. A pipe is a kernel byte buffer with a read end
and a write end; the reader sees EOF only when *every* copy of the write end
in *every* process is closed. Since `fork` duplicates the fd table, each
`fork` multiplies the outstanding copies — so a pipeline is mostly an
exercise in closing file descriptors: each child `dup2`s the end it needs
onto stdin/stdout and closes both originals, and the parent closes everything
it isn't reading from. One forgotten write end and the downstream command
hangs forever waiting for EOF (the single most common pipeline bug). Every
child must also be reaped with `waitpid`, or zombies accumulate. `trim_ws`
exists because command output arrives with incidental whitespace (e.g.
`wc -l` pads with spaces) and the tests compare trimmed strings.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `trim_ws` | Strips leading/trailing ASCII whitespace from `s` in place (via `memmove`) and returns `s` | In-place: shift the survivors to the front and keep NUL-termination |
| `pipeline2` | Runs `argv1 \| argv2` with two children joined by a pipe; captures child 2's stdout via a second pipe; 0 iff both exit 0 | Close every unused pipe end in every process; reap *both* children; `argv[0]` is an absolute path (plain `execv`, no PATH search) |
| `pipeline3` | Stretch goal: `argv1 \| argv2 \| argv3`, same contract, capturing the last command's stdout | Same discipline scaled up: two pipes between stages, three children to reap |

## signals

**Purpose** — Modern signal handling: `sigaction` installation, blocking and
pending signals, non-lossy SIGCHLD reaping, and the self-pipe trick for
waiting on a signal without races.

**Background** — APUE ch.10. A signal is asynchronous delivery: the kernel
interrupts the process at an arbitrary instruction and runs the handler on
top of whatever was executing — which is why handlers may only touch
`volatile sig_atomic_t` flags and call async-signal-safe functions
(`write(2)` is on the list; `printf` and `malloc` are not, since the
interrupted code may hold their internal locks). `sigaction` beats the
historical `signal(3)` because its semantics don't vary across systems, and
`SA_RESTART` makes slow syscalls resume instead of failing with `EINTR`.
Standard signals *do not queue*: three children dying in quick succession may
produce one SIGCHLD, so a correct reaper loops `waitpid(-1, ..., WNOHANG)`
until it returns 0. Finally, "check a flag, then sleep" has an unfixable
window where the signal lands between the check and the sleep; the self-pipe
trick closes it by converting the signal into a byte on a pipe, turning
signal delivery into ordinary fd readiness that `poll(2)` can wait on
atomically.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `install_handler` | Installs `fn` for `signo` via `sigaction` with `SA_RESTART` and an empty `sa_mask` | `sigaction`, never `signal(3)` — the latter's semantics vary across systems |
| `block_signal` / `unblock_signal` | Adds/removes `signo` in the process signal mask via `sigprocmask` | A blocked signal stays *pending* and is delivered on unblock — it isn't lost |
| `reap_children_count` | Forks 3 children, reaps them driven by a SIGCHLD handler that only sets a flag; returns 3 | One SIGCHLD may stand for several dead children — loop `waitpid(..., WNOHANG)` until 0 |
| `selfpipe_init` | Creates a nonblocking pipe and installs a SIGUSR1 handler that writes one byte to it | The handler does exactly one async-signal-safe thing: `write` of a single byte |
| `selfpipe_wait` | `poll`s the read end for up to `timeout_ms`; drains and returns 1 if a byte arrived, 0 on timeout | Turns a race-prone "wait for signal" into race-free fd readiness |

## threads

**Purpose** — Core pthread synchronization: a mutex-protected counter, a
bounded blocking queue with two condition variables, and a minimal thread
pool with clean shutdown.

**Background** — APUE ch.11–12. Threads share one address space, so an
unprotected `counter++` is a data race — a non-atomic read-modify-write that
loses updates and is undefined behavior, which is why the counter test
demands *exactly* `nthreads * increments_each`. Condition variables are how
threads sleep until a predicate becomes true: the waiter holds the mutex,
checks the predicate, and `pthread_cond_wait` atomically releases the mutex
and puts the thread on the kernel's wait queue; a `signal` from another
thread makes the scheduler wake it, and it re-acquires the mutex before
returning. Waits *must* sit in a `while` loop re-checking the predicate —
spurious wakeups are permitted by the spec, and another thread can steal the
condition between wakeup and re-acquisition. The bounded queue is the classic
producer/consumer of APUE §11.6 (one mutex, `not_full` + `not_empty`), and
the thread pool is its natural application: workers pull tasks off a shared
queue and a sentinel value or stop flag makes them exit for a clean
`pthread_join`. Value-equality tests can pass a racy program by luck —
`make test_threads SAN=thread` is the run that actually proves the locking.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `mutex_counter_run` | Spawns `nthreads` threads each doing `increments_each` locked increments; joins and returns the exact total | Unlocked `counter++` is a lost-update data race; the mutex makes the result deterministic |
| `bq_init` / `bq_destroy` | Initialize a `BoundedQueue` with capacity `cap`; free the buffer and destroy mutex/condvars | Two condvars (`not_full`, `not_empty`), not one — producers and consumers wait on different predicates |
| `bq_push` / `bq_pop` | Blocking push (waits while full) and pop (waits while empty) | `pthread_cond_wait` in a `while` loop — spurious and stolen wakeups are real |
| `tpool_run` | Builds a pool of `nthreads` workers, runs `ntasks` counter-increment tasks, shuts down cleanly, returns the count | Workers must exit their loop (sentinels or stop flag) so every `pthread_join` returns |

## atomics

**Purpose** — C11 atomics without locks: a spinlock built on `atomic_flag`
and a lock-free single-producer/single-consumer ring buffer with
acquire/release ordering.

**Background** — Phase 4.2 material. Below mutexes sits the C11 memory
model: by default both compilers and CPUs may reorder memory operations, and
atomics are how you constrain that. A *release* store publishes everything
written before it; an *acquire* load on the same variable makes those writes
visible — this pairing is the whole trick of the SPSC ring: the producer
writes the data slot, then release-stores `tail`; the consumer acquire-loads
`tail` and is guaranteed to see the slot's contents. Ownership is split so no
index is ever written by two threads (`tail` producer-only, `head`
consumer-only), which is what makes the design lock-free with plain loads and
stores — and precisely why it breaks the instant a second producer or
consumer appears. The power-of-two capacity lets indices wrap with a mask
instead of a modulo, and one slot is sacrificed so that full and empty are
distinguishable. The spinlock is the degenerate case: `atomic_flag`
test-and-set with acquire on lock, clear with release on unlock — a
busy-wait, so only sensible for tiny critical sections. As with `threads`,
ThreadSanitizer (`make test_atomics SAN=thread`) is what actually proves the
ordering.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `spin_lock` / `spin_unlock` | Busy-waits on `atomic_flag_test_and_set` (acquire) until it wins; clear (release) to unlock | No syscalls at all — it burns CPU, which is the trade-off vs. a mutex |
| `spsc_init` / `spsc_destroy` | Allocates a ring of capacity `cap` (must be a power of two, > 1); frees it | Usable capacity is `cap - 1`: one slot is sacrificed to tell full from empty |
| `spsc_push` | Producer-only: writes the slot, then release-stores `tail`; `false` when full | Data must be written *before* the release store publishes the slot |
| `spsc_pop` | Consumer-only: acquire-loads `tail`, reads the slot, advances `head`; `false` when empty | Acquire/release pairing is what makes the data visible — exactly one producer and one consumer, ever |

**Related:** the same SPSC ring shape reappears as the event-queue idea
behind ProperNet's `mux` poll loop, and the bounded queue in `threads` is the
locked equivalent of this structure.

## mmapio

**Purpose** — Memory-mapped file I/O: read a file through a private mapping,
mutate one in place through a shared mapping, and handle the empty-file trap.

**Background** — APUE ch.14. `mmap` doesn't copy the file — it wires a range
of your virtual address space directly to the kernel's *page cache*, the same
cache that `read`/`write` go through. Pages are faulted in lazily on first
touch, so mapping a huge file is cheap until you actually walk it. The
`MAP_PRIVATE` vs `MAP_SHARED` distinction is everything: private mappings
give you copy-on-write pages whose modifications never reach the file, while
shared writable mappings turn plain stores into file writes (flushed by the
kernel eventually, or forced with `msync(MS_SYNC)`). Two classic traps:
`mmap` with length 0 fails with `EINVAL`, so an empty file must be
special-cased rather than mapped; and touching pages beyond the file's
current size delivers `SIGBUS`. Both functions must also unmap and close on
every path — a mapping holds the file open even after `close`.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `mmap_count_byte` | Maps the file `PROT_READ`/`MAP_PRIVATE` and counts bytes equal to `b`; unmaps and closes before returning | Empty file → return 0 *without* calling `mmap` (length 0 is `EINVAL`) |
| `mmap_upper_inplace` | Maps `PROT_READ\|PROT_WRITE`/`MAP_SHARED`, uppercases `a..z` in place, `msync(MS_SYNC)`, unmaps | `MAP_SHARED` is what makes stores hit the file; empty file is a no-op success |

## fsmeta

**Purpose** — Filesystem metadata: recursive directory traversal with
`opendir`/`readdir`, precise file/dir counting with `lstat`, disk-usage
summing, and hard-link detection.

**Background** — APUE ch.4. A directory is just a table mapping names to
inode numbers; the inode holds the actual metadata (`st_mode`, `st_size`,
link count, owner). `readdir` hands you names, so classifying each entry
takes a `stat` call — and the `stat` vs `lstat` choice is the crux: `stat`
follows symlinks and reports on the *target*, `lstat` reports on the link
itself. A traversal that uses `stat` can be lured through a symlink into
counting things outside the tree (or looping forever), which is why this
module mandates `lstat` and counts symlinks as neither files nor directories.
Every directory scan must skip `.` and `..` or recursion never terminates.
Hard links are the other inode lesson: several names can map to one inode, so
"same file" means equal `st_dev` *and* equal `st_ino` — an inode number is
only unique within one filesystem, hence the device check. This is also how
real `du` avoids double-counting.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `walk_tree` | Recursively counts regular files (`*nfiles`) and directories (`*ndirs`, excluding `root` itself) under `root` | `lstat`, skip `.`/`..`; symlinks are not followed and count as neither |
| `du_bytes` | Sums `st_size` over every regular file under `root`, recursively | Same `lstat` discipline; only *regular* files contribute |
| `is_same_file` | 1 iff `a` and `b` name the same inode: equal `st_dev` **and** `st_ino` (via `stat`) | Inode numbers repeat across filesystems — the device check is not optional |

## procfs

**Purpose** — Read this process's own kernel-side state from Linux's `/proc`:
count memory mappings from `/proc/self/maps` and extract fields from
`/proc/self/status`.

**Background** — Linux-only, TLPI ch.12 flavor (APUE has no direct
equivalent — `/proc` is a Linux-ism). `/proc` is a virtual filesystem: the
files consume no disk and are synthesized by the kernel at `read` time, so
reading `/proc/self/maps` is literally asking the kernel to serialize your
current VM area list — one line per mapping (text segment, heap, each shared
library, stack, ...). `/proc/self/status` is the human-readable summary of
the process (name, state, VM sizes, signal masks) in `Field:\tvalue` lines.
The parsing gotchas are textual: match the field name exactly against the
part before the `:`, strip the leading whitespace of the value, drop the
trailing newline. Since macOS has no `/proc`, both functions compile
everywhere but return -1 off-Linux, and the test prints
`SKIP: procfs is Linux-only` and passes — the one module where "green on
macOS" means skipped, not verified.

**Functions**

| Function | What it does | Key gotcha/concept |
| --- | --- | --- |
| `count_maps_regions` | Counts lines of `/proc/self/maps` (one line = one memory mapping); > 0 for any live process | The file is generated on the fly by the kernel — its "size" is 0, so read it line by line |
| `read_status_field` | Finds `field` in `/proc/self/status` and copies its cleaned-up value into `out` (NUL-terminated) | Exact match on the name before `:`; strip leading whitespace and the trailing newline; -1 on non-Linux |
