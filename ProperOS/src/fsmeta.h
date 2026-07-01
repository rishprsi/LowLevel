#ifndef PROPEROS_FSMETA_H
#define PROPEROS_FSMETA_H

/*
 * APUE ch.4 — files and directories: stat, directory traversal, links.
 */

/*
 * Recursively walk the tree rooted at `root` with opendir/readdir/closedir,
 * skipping the "." and ".." entries.
 *
 * Counting rules (be precise — the tests are):
 * - *nfiles = number of REGULAR files anywhere under root.
 * - *ndirs  = number of directories under root INCLUDING nested subdirs,
 *   but NOT counting root itself.
 * - Use lstat(2): symlinks are NOT followed and count as neither files nor
 *   directories.
 * Returns 0 on success, -1 if root can't be opened as a directory.
 */
int walk_tree(const char *root, long *nfiles, long *ndirs);

/*
 * Total "disk usage": the sum of st_size over every regular file under root
 * (recursive, lstat, symlinks not followed). Returns the sum, or -1 if root
 * can't be opened.
 */
long du_bytes(const char *root);

/*
 * True (1) iff a and b name the SAME underlying file: equal st_dev AND equal
 * st_ino (this is how you detect hard links to one inode). Uses stat(2).
 * Returns 0 if different files, -1 if either can't be stat'ed.
 */
int is_same_file(const char *a, const char *b);

#endif /* PROPEROS_FSMETA_H */
