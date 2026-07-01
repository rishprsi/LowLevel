#ifndef PROPEROS_FILEIO_H
#define PROPEROS_FILEIO_H

#include <stddef.h>
#include <sys/types.h>

/*
 * APUE ch.3 — unbuffered file I/O.
 *
 * read(2)/write(2) may transfer FEWER bytes than asked (partial transfer) or
 * fail with errno == EINTR when a signal interrupts them. Robust code loops.
 */

/*
 * Read exactly n bytes from fd into buf, looping over partial reads and
 * retrying on EINTR. Returns the number of bytes actually read — which is
 * less than n only if EOF was hit first — or -1 on a real read error.
 */
ssize_t read_all(int fd, void *buf, size_t n);

/*
 * Write exactly n bytes from buf to fd, looping over partial writes and
 * retrying on EINTR. Returns n on success, -1 on a real write error.
 */
ssize_t write_all(int fd, const void *buf, size_t n);

/*
 * Copy the file at src to dst (created/truncated with mode 0644), using
 * read_all/write_all over a fixed-size buffer. Must be binary-safe (embedded
 * NUL bytes copied faithfully). Returns 0 on success, -1 on any error.
 */
int copy_file(const char *src, const char *dst);

/*
 * Return the size in bytes of the file open on fd, computed with lseek(2)
 * only (no stat). The file offset that was current on entry MUST be restored
 * before returning. Returns -1 on error.
 */
long file_size_fd(int fd);

#endif /* PROPEROS_FILEIO_H */
