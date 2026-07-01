#include "ctest.h"
#include "sockets.h"

#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Client thread: connect to the given port, dribble "hello\n" one byte at a
 * time (with small pauses, to force the server to reassemble across many
 * reads), then verify we get the exact echo back.
 */
typedef struct {
    uint16_t port;
    int ok;
} ClientArgs;

static void *client_main(void *arg) {
    ClientArgs *ca = arg;
    ca->ok = 0;

    int fd = tcp_connect_loopback(ca->port);
    if (fd < 0) {
        return NULL;
    }
    const char *msg = "hello\n";
    size_t msglen = strlen(msg);
    for (size_t i = 0; i < msglen; i++) {
        if (send_all(fd, msg + i, 1) != 1) {
            close(fd);
            return NULL;
        }
        usleep(2000); /* let each byte arrive on its own */
    }
    char buf[64];
    ssize_t n = recv_until(fd, buf, sizeof buf, '\n');
    if (n == (ssize_t)msglen && memcmp(buf, msg, msglen) == 0) {
        ca->ok = 1;
    }
    close(fd);
    return NULL;
}

int main(void) {
    /* --- send_all / recv_until semantics on a socketpair --- */
    int sp[2];
    CHECK_INT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sp), 0);
    CHECK_INT_EQ((int)send_all(sp[0], "abc\ndef", 7), 7);

    char buf[16];
    /* stops at (and includes) the delimiter */
    CHECK_INT_EQ((int)recv_until(sp[1], buf, sizeof buf, '\n'), 4);
    CHECK(memcmp(buf, "abc\n", 4) == 0);
    /* stops at cap, must not consume more than cap bytes */
    CHECK_INT_EQ((int)recv_until(sp[1], buf, 2, '\n'), 2);
    CHECK(memcmp(buf, "de", 2) == 0);
    close(sp[0]); /* EOF for whatever remains */
    CHECK_INT_EQ((int)recv_until(sp[1], buf, sizeof buf, '\n'), 1);
    CHECK(buf[0] == 'f');
    /* EOF with nothing buffered -> 0 */
    CHECK_INT_EQ((int)recv_until(sp[1], buf, sizeof buf, '\n'), 0);
    close(sp[1]);

    /* --- real loopback TCP, 20 rounds: a leaked fd per round would
     *     eventually make tcp_listen_loopback start failing --- */
    for (int iter = 0; iter < 20; iter++) {
        uint16_t port = 0; /* 0 = kernel picks; getsockname must fill it in */
        int lfd = tcp_listen_loopback(&port);
        CHECK(lfd >= 0);
        CHECK(port != 0);

        ClientArgs ca = { .port = port, .ok = 0 };
        pthread_t th;
        CHECK_INT_EQ(pthread_create(&th, NULL, client_main, &ca), 0);

        int cfd = accept(lfd, NULL, NULL);
        CHECK(cfd >= 0);

        /* the client sends 1 byte at a time; recv_until must reassemble */
        char line[64];
        ssize_t n = recv_until(cfd, line, sizeof line, '\n');
        CHECK_INT_EQ((int)n, 6);
        CHECK(n == 6 && memcmp(line, "hello\n", 6) == 0);

        /* echo it back in one go */
        CHECK_INT_EQ((int)send_all(cfd, line, (size_t)n), 6);

        pthread_join(th, NULL);
        CHECK_INT_EQ(ca.ok, 1);

        close(cfd);
        close(lfd);
    }

    /* one more listener must still succeed after all those rounds */
    uint16_t port = 0;
    int lfd = tcp_listen_loopback(&port);
    CHECK(lfd >= 0);
    close(lfd);

    CTEST_END();
}
