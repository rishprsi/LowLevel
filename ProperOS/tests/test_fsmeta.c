#include "ctest.h"
#include "fsmeta.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int put_file(const char *dir, const char *name, size_t nbytes) {
    char path[PATH_MAX];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return -1;
    }
    for (size_t i = 0; i < nbytes; i++) {
        if (write(fd, "x", 1) != 1) {
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}

int main(void) {
    /*
     * Fixture tree (mkdtemp):
     *   root/
     *     a.txt          (10 bytes)
     *     b.bin          (100 bytes)
     *     sub1/
     *       c.txt        (7 bytes)
     *       deep/
     *         d.txt      (3 bytes)
     *     sub2/          (empty dir)
     *     link_to_a      (symlink -> a.txt; must NOT be counted or followed)
     *   => nfiles = 4, ndirs = 3 (sub1, sub1/deep, sub2), du = 120
     */
    char root[] = "/tmp/properos_fsmeta_XXXXXX";
    CHECK_PTR_NONNULL(mkdtemp(root));

    char sub1[PATH_MAX], deep[PATH_MAX], sub2[PATH_MAX];
    snprintf(sub1, sizeof sub1, "%s/sub1", root);
    snprintf(deep, sizeof deep, "%s/sub1/deep", root);
    snprintf(sub2, sizeof sub2, "%s/sub2", root);
    CHECK_INT_EQ(mkdir(sub1, 0755), 0);
    CHECK_INT_EQ(mkdir(deep, 0755), 0);
    CHECK_INT_EQ(mkdir(sub2, 0755), 0);

    CHECK_INT_EQ(put_file(root, "a.txt", 10), 0);
    CHECK_INT_EQ(put_file(root, "b.bin", 100), 0);
    CHECK_INT_EQ(put_file(sub1, "c.txt", 7), 0);
    CHECK_INT_EQ(put_file(deep, "d.txt", 3), 0);

    char target[PATH_MAX], linkpath[PATH_MAX];
    snprintf(target, sizeof target, "%s/a.txt", root);
    snprintf(linkpath, sizeof linkpath, "%s/link_to_a", root);
    CHECK_INT_EQ(symlink(target, linkpath), 0);

    /* walk_tree */
    long nfiles = -1, ndirs = -1;
    CHECK_INT_EQ(walk_tree(root, &nfiles, &ndirs), 0);
    CHECK_INT_EQ(nfiles, 4);
    CHECK_INT_EQ(ndirs, 3);

    /* an empty dir walks fine */
    CHECK_INT_EQ(walk_tree(sub2, &nfiles, &ndirs), 0);
    CHECK_INT_EQ(nfiles, 0);
    CHECK_INT_EQ(ndirs, 0);

    /* error: root doesn't exist */
    CHECK_INT_EQ(walk_tree("/nonexistent/no/dir", &nfiles, &ndirs), -1);

    /* du_bytes */
    CHECK_INT_EQ(du_bytes(root), 120);
    CHECK_INT_EQ(du_bytes(sub2), 0);
    CHECK_INT_EQ(du_bytes("/nonexistent/no/dir"), -1);

    /* is_same_file with a hard link */
    char hard[PATH_MAX];
    snprintf(hard, sizeof hard, "%s/hardlink_to_a", root);
    CHECK_INT_EQ(link(target, hard), 0);
    CHECK_INT_EQ(is_same_file(target, hard), 1);

    char other[PATH_MAX];
    snprintf(other, sizeof other, "%s/b.bin", root);
    CHECK_INT_EQ(is_same_file(target, other), 0);
    CHECK_INT_EQ(is_same_file(target, "/nonexistent/no/file"), -1);

    /* the hard link bumped nfiles: both names are regular files */
    CHECK_INT_EQ(walk_tree(root, &nfiles, &ndirs), 0);
    CHECK_INT_EQ(nfiles, 5);
    CHECK_INT_EQ(du_bytes(root), 130);

    /* clean up the fixture */
    unlink(hard);
    unlink(linkpath);
    unlink(target);
    char p[PATH_MAX];
    snprintf(p, sizeof p, "%s/b.bin", root);
    unlink(p);
    snprintf(p, sizeof p, "%s/sub1/c.txt", root);
    unlink(p);
    snprintf(p, sizeof p, "%s/sub1/deep/d.txt", root);
    unlink(p);
    rmdir(deep);
    rmdir(sub1);
    rmdir(sub2);
    rmdir(root);

    CTEST_END();
}
