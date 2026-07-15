# Drill 01 — Processes, /proc, strace (45 min)

**Goal:** Narrate what the kernel does when a shell runs a pipeline, walk `/proc` like
a map, find open files two ways, and explain D-state / load average like a support
engineer, not a textbook.

Install what you need first:

```bash
sudo apt update && sudo apt install -y strace lsof htop
```

---

## Part 1 — strace a pipeline and narrate every syscall group

### Step 1: run the trace

```bash
strace -f -e trace=%process,%file sh -c 'ls | wc -l' 2>&1 | less
```

`-f` follows children (essential — the interesting work happens in forked children).
`%process` = fork/exec/exit/wait family, `%file` = open/stat/access family.

**Say aloud:** "I'm tracing the process-lifecycle and file syscalls of a shell
running a two-command pipeline, following forks."

### Step 2: find and narrate these events, in order

1. **`clone(...)` twice** (modern glibc uses `clone`, not literal `fork`). One child
   per side of the pipe.
   **Say aloud:** "The shell forks twice — one child for `ls`, one for `wc`. The pipe
   was created *before* the forks so both children inherit its file descriptors."
2. **`pipe2([3, 4], ...)`** in the parent, before the clones.
   **Say aloud:** "fd 3 is the read end, fd 4 the write end."
3. **`dup2(4, 1)`** (or `dup3`) in the `ls` child — write end becomes stdout.
   **`dup2(3, 0)`** in the `wc` child — read end becomes stdin.
   **Say aloud:** "Redirection is just fd surgery before exec. The exec'd program
   never knows it's in a pipeline — it just reads fd 0 and writes fd 1."
4. **`execve("/usr/bin/ls", ...)`** and **`execve("/usr/bin/wc", ...)`** — note the
   preceding failed `access`/`newfstatat` calls as the shell walks `$PATH`.
5. **`wait4(...)`** in the parent, and the children's exit statuses.
   **Say aloud:** "The parent reaps both children; that's why there are no zombies."

### Step 3: two more strace tricks worth showing off

```bash
strace -c -f sh -c 'ls | wc -l'        # syscall count/time summary table
strace -p <PID> -e trace=%network      # attach to a live process (needs sudo or same user)
```

**Say aloud:** "In production I'd attach with `-p` sparingly — strace stops the
process at every syscall, so it can badly slow a hot service. For lower overhead
I'd reach for `perf trace` or bpftrace."

---

## Part 2 — /proc walk

Pick a victim:

```bash
sleep 600 &
P=$!
```

Walk these, and narrate what each is *for*:

```bash
cat /proc/$P/status      # human-readable: State, VmRSS, Threads, UID, capabilities
cat /proc/$P/cmdline | tr '\0' ' '; echo   # argv, NUL-separated (ps reads this)
ls -l /proc/$P/fd        # open fds as symlinks — pipes, sockets, files
cat /proc/$P/maps | head -20   # memory map: binary, libs, heap, stack, anon
cat /proc/$P/cgroup      # which cgroup — on 24.04 a systemd slice/scope path
ls -l /proc/$P/exe /proc/$P/cwd   # the running binary and working dir (symlinks)
```

**Say aloud, per file:**
- `status` — "State field: R running, S sleeping, D uninterruptible, Z zombie, T stopped.
  VmRSS is the real physical memory answer, not VSZ."
- `fd` — "This is ground truth for open files — lsof is basically a pretty-printer
  over this."
- `maps` — "If I suspect a memory leak I diff the heap and anon regions over time."
- `cgroup` — "On Ubuntu 24.04 it's cgroup v2, unified hierarchy; every service lives
  in its own slice so systemd can account and limit CPU/memory per unit."
- `exe` — "If this symlink says `(deleted)`, the binary was replaced on disk after
  start — classic 'we upgraded the package but the old code is still running'."

Clean up: `kill $P`

---

## Part 3 — open files: lsof and /proc, both directions

```bash
lsof -p $P                    # everything this process has open
lsof /var/log/syslog          # who has THIS file open
sudo lsof -i :22              # who owns this port (ss -tlnp is the modern habit)
sudo ls -l /proc/*/fd 2>/dev/null | grep deleted   # deleted-but-open files, no lsof needed
```

**Say aloud:** "Two directions: process→files with `lsof -p` or `/proc/PID/fd`, and
file→processes with `lsof <path>`. And if lsof isn't installed on a minimal box,
/proc gives me everything it knows."

---

## Part 4 — D-state and load average (the classic trap)

Load average = the average number of tasks that are **runnable OR in uninterruptible
sleep (D)**. Not a CPU percentage.

**Say aloud:** "High load with low CPU almost always means processes stuck in D-state —
uninterruptible sleep, usually waiting on I/O: a dying disk, an unreachable NFS server,
a saturated block device. They count toward load but burn no CPU."

Verify tools:

```bash
uptime                              # the three load numbers: 1/5/15 min
ps -eo pid,stat,wchan:30,comm | awk '$2 ~ /D/'   # find D-state tasks + what they wait on
vmstat 1 5                          # b column = blocked (D) tasks; wa = iowait CPU%
iostat -x 1 3                       # per-device %util and await (apt install sysstat)
```

**Say aloud:** "`wchan` tells me *which kernel function* they're sleeping in — if I see
NFS or block-layer symbols, I know where to look next. You can't `kill -9` a D-state
process; it won't respond until the I/O completes or times out. The fix is the
underlying device, not the process."

---

## Part 5 — ps / top / htop field reading

```bash
ps aux --sort=-%mem | head
top    # then press: 1 (per-CPU), M (sort mem), P (sort cpu), c (full cmdline)
htop   # tree view with F5
```

Fields to be fluent in:
- **VSZ vs RSS** — virtual address space vs resident physical memory. RSS is what
  matters for "is it eating my RAM" (with the caveat of shared pages).
- **STAT** — `R`,`S`,`D`,`Z`,`T` plus suffixes: `s` session leader, `+` foreground,
  `l` multi-threaded, `<` high priority, `N` niced.
- **top header** — `us/sy/ni/id/wa/hi/si/st`.
  **Say aloud:** "`wa` high → I/O bound. `sy` high → lots of kernel work, maybe
  syscall-heavy or a driver problem. `st` (steal) high on a VM → the *hypervisor* is
  overcommitted; nothing inside the guest will fix it."
- **Zombie (Z)** — already dead, only a process-table entry; fix the *parent* that
  isn't calling wait(), don't try to kill the zombie.

---

## Self-check questions

1. In the strace output, why is `pipe2` called before the two `clone` calls, and why
   does that ordering matter?
2. Your traced shell shows `execve("/usr/local/bin/ls", ...) = -1 ENOENT` before the
   successful execve. What is happening?
3. What's the difference between `/proc/PID/cmdline` and `/proc/PID/comm`, and which
   one can a process spoof more easily?
4. `df` says a filesystem is 95% full but `du` on it sums to 60%. Using only /proc,
   how do you find the culprit?
5. Load average is 48 on a 4-core box, but `top` shows 92% idle. Give the most likely
   cause and the two commands you'd run next.
6. Why can't you kill a D-state process with SIGKILL, and what's the actual remedy?
7. What does it mean when `/proc/PID/exe` points to a path suffixed `(deleted)`, and
   when does this commonly happen on a server?
8. In top, `st` is at 30%. What do you tell the customer?

<details>
<summary><strong>Answers</strong></summary>

1. Both children must inherit the pipe's fds; fds are copied at fork time, so the pipe
   must exist before the forks. Create it after and the children can't share it.
2. The shell is walking `$PATH` entry by entry; each miss returns ENOENT until it finds
   the binary. (Bash typically probes with stat/access; a raw execve loop is what
   `execvp` does.)
3. `cmdline` is the full argv (NUL-separated); `comm` is the 15-char task name.
   A process can overwrite its argv (so `cmdline` lies easily) and can also set comm
   via prctl — cross-check with `/proc/PID/exe`, which it cannot fake.
4. A process holds a deleted file open: `ls -l /proc/*/fd 2>/dev/null | grep deleted`
   (or `lsof +L1`). The space frees only when the fd closes; truncate via
   `/proc/PID/fd/N` if you can't restart the process.
5. Tasks stuck in D-state (uninterruptible I/O wait) — e.g. hung NFS mount or failing
   disk. Run `ps -eo pid,stat,wchan,comm | awk '$2~/D/'` and `iostat -x 1` (or
   `vmstat 1` to watch the `b` column).
6. It's inside an uninterruptible kernel code path (usually waiting for I/O completion);
   signals are only delivered when it returns to killable state. Remedy: fix or remove
   the underlying I/O source (remount, power-cycle the device, `umount -l` a dead NFS
   mount). Reboot as last resort.
7. The on-disk binary was deleted/replaced while the process runs — typical right after
   a package upgrade. It means the service is still executing the *old* code and needs
   a restart (this is what `needrestart` checks for).
8. Steal time: the hypervisor isn't scheduling this vCPU because the host is
   overcommitted. Nothing inside the guest fixes it — resize the instance or move it;
   raise it with the cloud/virt operator.

</details>
