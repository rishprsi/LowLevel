#include "ctest.h"
#include "fileio.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Make a temp file containing the given bytes; returns an fd positioned at 0.
 * Path is copied into pathbuf (must hold at least 64 bytes). */
static int make_temp(const void *data, size_t n, char *pathbuf) {
    strcpy(pathbuf, "/tmp/properos_fileio_XXXXXX");
    int fd = mkstemp(pathbuf);
    if (fd < 0) {
        return -1;
    }
    if (write_all(fd, data, n) != (ssize_t)n) {
        return -1;
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        return -1;
    }
    return fd;
}

int main(void) {
    char path1[64], path2[64], path3[64];

    /* read_all / write_all round-trip */
    SECTION("read_all / write_all");
    const char msg[] = "hello, file i/o";
    int fd = make_temp(msg, sizeof msg - 1, path1);
    CHECK_TRUE(fd >= 0);

    char buf[64];
    memset(buf, 0, sizeof buf);
    CHECK_INT_EQ(read_all(fd, buf, sizeof msg - 1), 999L);
    CHECK_TRUE(memcmp(buf, msg, sizeof msg - 1) == 0);

    /* read_all past EOF: returns what was available (0 here) */
    CHECK_INT_EQ(read_all(fd, buf, 16), 0);

    /* asking for more than the file holds: short count, not an error */
    CHECK_TRUE(lseek(fd, 0, SEEK_SET) == 0);
    CHECK_INT_EQ(read_all(fd, buf, sizeof buf), (long)(sizeof msg - 1));

    /* file_size_fd preserves the current offset */
    SECTION("file_size_fd");
    CHECK_TRUE(lseek(fd, 5, SEEK_SET) == 5);
    CHECK_INT_EQ(file_size_fd(fd), (long)(sizeof msg - 1));
    CHECK_INT_EQ((long)lseek(fd, 0, SEEK_CUR), 5);
    close(fd);
    unlink(path1);

    /* copy_file: binary-safe (embedded NUL bytes) */
    SECTION("copy_file");
    const unsigned char blob[] = {'a', 0, 'b', 0, 0, 'c', 255, 128, 0, 'z'};
    fd = make_temp(blob, sizeof blob, path2);
    CHECK_TRUE(fd >= 0);
    close(fd);

    strcpy(path3, "/tmp/properos_fileio_dst_XXXXXX");
    int dfd = mkstemp(path3);
    CHECK_TRUE(dfd >= 0);
    close(dfd);

    CHECK_INT_EQ(copy_file(path2, path3), 0);

    int cfd = open(path3, O_RDONLY);
    CHECK_TRUE(cfd >= 0);
    unsigned char got[sizeof blob + 4];
    ssize_t r = read_all(cfd, got, sizeof got);
    CHECK_INT_EQ(r, (long)sizeof blob);
    CHECK_TRUE(memcmp(got, blob, sizeof blob) == 0);
    CHECK_INT_EQ(file_size_fd(cfd), (long)sizeof blob);
    close(cfd);

    /* copy_file with a bigger-than-one-buffer file (forces looping) */
    SECTION("copy_file large");
    size_t big_n = 100 * 1024 + 37;
    unsigned char *big = malloc(big_n);
    CHECK_PTR_NONNULL(big);
    for (size_t i = 0; i < big_n; i++) {
        big[i] = (unsigned char)(i * 31 + 7);
    }
    fd = make_temp(big, big_n, path1);
    CHECK_TRUE(fd >= 0);
    close(fd);
    CHECK_INT_EQ(copy_file(path1, path3), 0);
    cfd = open(path3, O_RDONLY);
    CHECK_TRUE(cfd >= 0);
    CHECK_INT_EQ(file_size_fd(cfd), (long)big_n);
    unsigned char *back = malloc(big_n);
    CHECK_PTR_NONNULL(back);
    CHECK_INT_EQ(read_all(cfd, back, big_n), (long)big_n);
    CHECK_TRUE(memcmp(back, big, big_n) == 0);
    close(cfd);
    free(big);
    free(back);

    /* copy_file error path: nonexistent source */
    SECTION("copy_file errors");
    CHECK_INT_EQ(copy_file("/nonexistent/no/such/file", path3), -1);

    unlink(path1);
    unlink(path2);
    unlink(path3);

    CTEST_END();
}
