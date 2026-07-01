#include "sockets.h"
#include "todo.h"

ssize_t send_all(int fd, const void *buf, size_t n) {
    (void)fd;
    (void)buf;
    (void)n;
    TODO("implement send_all");
}

ssize_t recv_until(int fd, char *buf, size_t cap, char delim) {
    (void)fd;
    (void)buf;
    (void)cap;
    (void)delim;
    TODO("implement recv_until");
}

int tcp_listen_loopback(uint16_t *port_inout) {
    (void)port_inout;
    TODO("implement tcp_listen_loopback");
}

int tcp_connect_loopback(uint16_t port) {
    (void)port;
    TODO("implement tcp_connect_loopback");
}
