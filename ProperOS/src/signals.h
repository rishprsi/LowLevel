#ifndef PROPEROS_SIGNALS_H
#define PROPEROS_SIGNALS_H

/*
 * APUE ch.10 — signals: sigaction, blocking, SIGCHLD reaping, and the
 * self-pipe trick.
 *
 * Rules of the road: handlers may only touch volatile sig_atomic_t flags and
 * call async-signal-safe functions (write(2) is; printf is NOT).
 */

/*
 * Install fn as the handler for signo using sigaction(2) — NOT signal(3),
 * whose semantics vary across systems. Set SA_RESTART so slow syscalls are
 * restarted instead of failing with EINTR, and an empty sa_mask.
 * Returns 0 on success, -1 on error.
 */
int install_handler(int signo, void (*fn)(int));

/*
 * Add signo to / remove signo from this process's blocked-signal mask using
 * sigprocmask(2) with SIG_BLOCK / SIG_UNBLOCK. A blocked signal stays
 * pending and is delivered on unblock. Returns 0 on success, -1 on error.
 */
int block_signal(int signo);
int unblock_signal(int signo);

/*
 * SIGCHLD-driven reaping. Fork 3 short-lived children (each just _exit(0)s).
 * Install a SIGCHLD handler that ONLY sets a volatile sig_atomic_t flag.
 * In the main flow, loop: when the flag is set, clear it and reap with
 *     waitpid(-1, &st, WNOHANG)
 * until it returns 0 or -1 — one SIGCHLD may stand for SEVERAL dead
 * children (signals don't queue). Keep looping (a brief sleep or sigsuspend
 * between rounds is fine) until 3 children have been collected.
 * Returns the number of children reaped (3), or -1 on error.
 */
int reap_children_count(void);

/*
 * THE SELF-PIPE TRICK — the classic way to wait for a signal without racing.
 *
 * selfpipe_init: create a pipe, make BOTH ends nonblocking (O_NONBLOCK via
 * fcntl F_SETFL), store {read,write} in pipefd, and remember the write end
 * in a file-scope variable so a signal handler can reach it. Install an
 * internal SIGUSR1 handler (via install_handler) that does exactly one
 * async-signal-safe thing: write one byte to the write end.
 * Returns 0 on success, -1 on error.
 *
 * selfpipe_wait: block in poll(2) on readfd (POLLIN) for up to timeout_ms
 * milliseconds. If a byte arrives, drain it with read(2) and return 1
 * ("signal happened"). If poll times out, return 0. On error, -1.
 */
int selfpipe_init(int pipefd[2]);
int selfpipe_wait(int readfd, int timeout_ms);

#endif /* PROPEROS_SIGNALS_H */
