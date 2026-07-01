#ifndef PROPEROS_MMAPIO_H
#define PROPEROS_MMAPIO_H

/*
 * APUE ch.14 — memory-mapped I/O with mmap(2).
 *
 * Gotcha both functions must handle: mmap with length 0 is INVALID (EINVAL).
 * An empty file must be special-cased, not mapped.
 */

/*
 * Map the file at path with PROT_READ / MAP_PRIVATE and count how many bytes
 * equal b. Returns the count (0 for an empty file — do NOT call mmap then),
 * or -1 on error. Unmap and close before returning.
 */
long mmap_count_byte(const char *path, unsigned char b);

/*
 * Map the file at path with PROT_READ|PROT_WRITE and MAP_SHARED (so stores
 * hit the file), uppercase every ASCII letter 'a'..'z' in place, then
 * msync(2) with MS_SYNC before unmapping. An empty file is a no-op success.
 * Returns 0 on success, -1 on error.
 */
int mmap_upper_inplace(const char *path);

#endif /* PROPEROS_MMAPIO_H */
