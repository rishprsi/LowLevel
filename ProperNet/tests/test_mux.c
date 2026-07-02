#include "ctest.h"
#include "mux.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* The test provides its own socket plumbing so this module only depends on
 * the learner's echo_poll_serve. */
static int make_listener(uint16_t *port_out) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = 0; /* ephemeral */
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(fd, (const struct sockaddr *)&addr, sizeof addr) < 0 ||
        listen(fd, 16) < 0) {
        close(fd);
        return -1;
    }
    socklen_t alen = sizeof addr;
    if (getsockname(fd, (struct sockaddr *)&addr, &alen) < 0) {
        close(fd);
        return -1;
    }
    *port_out = ntohs(addr.sin_port);
    return fd;
}

static int connect_loopback(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (const struct sockaddr *)&addr, sizeof addr) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

enum { NCLIENTS = 3, NMSGS = 4 };

/*
 * Each client sends NMSGS distinct messages, sleeping between them so the
 * three clients interleave on the server, and after each send verifies it
 * reads back EXACTLY its own bytes, in order.
 */
typedef struct {
    uint16_t port;
    int id;
    int ok;
} ClientArgs;

static void *client_main(void *arg) {
    ClientArgs *ca = arg;
    ca->ok = 0;

    int fd = connect_loopback(ca->port);
    if (fd < 0) {
        return NULL;
    }
    usleep((unsigned)(1000 * (ca->id + 1))); /* stagger the clients */

    for (int m = 0; m < NMSGS; m++) {
        char msg[64];
        int len = snprintf(msg, sizeof msg, "<client%d/message%d>", ca->id, m);
        if (write(fd, msg, (size_t)len) != (ssize_t)len) {
            close(fd);
            return NULL;
        }
        char back[64];
        size_t got = 0;
        while (got < (size_t)len) {
            ssize_t r = read(fd, back + got, (size_t)len - got);
            if (r <= 0) {
                close(fd);
                return NULL;
            }
            got += (size_t)r;
        }
        if (memcmp(back, msg, (size_t)len) != 0) {
            close(fd);
            return NULL; /* got somebody else's bytes! */
        }
        usleep(3000); /* leave room for the other clients to interleave */
    }
    close(fd);
    ca->ok = 1;
    return NULL;
}

int main(void) {
    /* bad arguments are rejected without touching the network */
    SECTION("echo_poll_serve bad args");
    CHECK_INT_EQ(echo_poll_serve(-1, 1), -1);
    CHECK_INT_EQ(echo_poll_serve(-1, 0), -1);
    CHECK_INT_EQ(echo_poll_serve(-1, MUX_MAX_CLIENTS + 1), -1);

    SECTION("echo_poll_serve concurrent clients");
    uint16_t port = 0;
    int lfd = make_listener(&port);
    CHECK(lfd >= 0);

    pthread_t th[NCLIENTS];
    ClientArgs args[NCLIENTS];
    for (int i = 0; i < NCLIENTS; i++) {
        args[i].port = port;
        args[i].id = i;
        args[i].ok = 0;
        CHECK_INT_EQ(pthread_create(&th[i], NULL, client_main, &args[i]), 0);
    }

    /* single-threaded poll loop serving all three concurrent clients */
    CHECK_INT_EQ(echo_poll_serve(lfd, NCLIENTS), 0);

    for (int i = 0; i < NCLIENTS; i++) {
        pthread_join(th[i], NULL);
        CHECK_INT_EQ(args[i].ok, 1);
    }
    close(lfd);

    CTEST_END();
}
