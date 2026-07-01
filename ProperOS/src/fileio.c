#include "fileio.h"
#include "todo.h"

ssize_t read_all(int fd, void *buf, size_t n) {
    (void)fd;
    (void)buf;
    (void)n;
    TODO("implement read_all");
}

ssize_t write_all(int fd, const void *buf, size_t n) {
    (void)fd;
    (void)buf;
    (void)n;
    TODO("implement write_all");
}

int copy_file(const char *src, const char *dst) {
    (void)src;
    (void)dst;
    TODO("implement copy_file");
}

long file_size_fd(int fd) {
    (void)fd;
    TODO("implement file_size_fd");
}
