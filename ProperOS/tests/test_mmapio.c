#include "ctest.h"
#include "mmapio.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int make_temp(const void *data, size_t n, char *pathbuf) {
    strcpy(pathbuf, "/tmp/properos_mmapio_XXXXXX");
    int fd = mkstemp(pathbuf);
    if (fd < 0) {
        return -1;
    }
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(fd, (const char *)data + off, n - off);
        if (w < 0) {
            close(fd);
            return -1;
        }
        off += (size_t)w;
    }
    close(fd);
    return 0;
}

/* read the whole (small) file back with plain read(2) to verify mmap writes */
static long read_back(const char *path, char *buf, size_t bufsz) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    size_t total = 0;
    for (;;) {
        ssize_t r = read(fd, buf + total, bufsz - 1 - total);
        if (r < 0) {
            close(fd);
            return -1;
        }
        if (r == 0) {
            break;
        }
        total += (size_t)r;
    }
    close(fd);
    buf[total] = '\0';
    return (long)total;
}

int main(void) {
    char path[64];

    /* mmap_count_byte on a known blob (includes NUL bytes) */
    {
        SECTION("mmap_count_byte");
        const unsigned char blob[] = {'a', 'b', 'a', 0, 'a', 0, 'x'};
        CHECK_INT_EQ(make_temp(blob, sizeof blob, path), 0);
        CHECK_INT_EQ(mmap_count_byte(path, 'a'), 3);
        CHECK_INT_EQ(mmap_count_byte(path, 0), 2);
        CHECK_INT_EQ(mmap_count_byte(path, 'z'), 0);
        unlink(path);
    }

    /* empty file: must return 0, not crash on mmap(len=0) */
    {
        SECTION("mmap empty file");
        CHECK_INT_EQ(make_temp("", 0, path), 0);
        CHECK_INT_EQ(mmap_count_byte(path, 'a'), 0);
        CHECK_INT_EQ(mmap_upper_inplace(path), 0); /* no-op success */
        unlink(path);
    }

    /* a file bigger than one page */
    {
        SECTION("mmap_count_byte large");
        size_t n = 3 * 4096 + 123;
        unsigned char *big = malloc(n);
        CHECK_PTR_NONNULL(big);
        long want = 0;
        for (size_t i = 0; i < n; i++) {
            big[i] = (unsigned char)(i % 251);
            if (big[i] == 42) {
                want++;
            }
        }
        CHECK_INT_EQ(make_temp(big, n, path), 0);
        CHECK_INT_EQ(mmap_count_byte(path, 42), want);
        free(big);
        unlink(path);
    }

    /* mmap_upper_inplace: verify through ordinary read(2) */
    {
        SECTION("mmap_upper_inplace");
        const char *text = "Hello, mmap World! 123 xyz";
        CHECK_INT_EQ(make_temp(text, strlen(text), path), 0);
        CHECK_INT_EQ(mmap_upper_inplace(path), 0);
        char buf[128];
        CHECK_INT_EQ(read_back(path, buf, sizeof buf), (long)strlen(text));
        CHECK_STR_EQ(buf, "HELLO, MMAP WORLD! 123 XYZ");
        /* idempotent */
        CHECK_INT_EQ(mmap_upper_inplace(path), 0);
        CHECK_INT_EQ(read_back(path, buf, sizeof buf), (long)strlen(text));
        CHECK_STR_EQ(buf, "HELLO, MMAP WORLD! 123 XYZ");
        unlink(path);
    }

    /* error paths */
    SECTION("mmap error paths");
    CHECK_INT_EQ(mmap_count_byte("/nonexistent/no/file", 'a'), -1);
    CHECK_INT_EQ(mmap_upper_inplace("/nonexistent/no/file"), -1);

    CTEST_END();
}
