#ifndef PROPERNET_SOCKETS_H
#define PROPERNET_SOCKETS_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/*
 * TCP building blocks. All of these are exercised against REAL loopback
 * sockets by the tests (a client pthread connects to your listener), so
 * "looks right" is not enough — partial reads/writes and fd hygiene matter.
 */

/*
 * Write ALL n bytes of buf to fd, looping until done. write(2) is allowed to
 * accept fewer bytes than asked (a partial write) — your job is to keep
 * going from where it left off. Retry on EINTR.
 * Returns (ssize_t)n on success, -1 on any write error.
 */
ssize_t send_all(int fd, const void *buf, size_t n);

/*
 * Read from fd into buf until either
 *   - a byte equal to delim has been stored (it IS included in the result),
 *   - cap bytes have been stored, or
 *   - EOF / error.
 * Reading byte-by-byte is fine here (simple and always correct); a buffered
 * version must be careful not to consume bytes past the delimiter.
 * Returns the number of bytes stored (0 means EOF before any byte),
 * or -1 on a read error. The result is NOT NUL-terminated.
 */
ssize_t recv_until(int fd, char *buf, size_t cap, char delim);

/*
 * Create a TCP listening socket bound to 127.0.0.1.
 * On entry *port_inout is the requested port (0 = let the kernel pick an
 * ephemeral port). On success *port_inout is updated with the ACTUAL bound
 * port (use getsockname(2)) — this is how tests avoid port collisions.
 * Set SO_REUSEADDR before binding. Use a backlog of at least 8.
 * Returns the listening fd, or -1 on error (leaking no fd on error paths!).
 */
int tcp_listen_loopback(uint16_t *port_inout);

/*
 * Connect to 127.0.0.1:port. Returns the connected fd, or -1 on error
 * (again: no fd leaks on error paths).
 */
int tcp_connect_loopback(uint16_t port);

#endif /* PROPERNET_SOCKETS_H */
