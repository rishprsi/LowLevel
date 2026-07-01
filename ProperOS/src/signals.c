#include "signals.h"
#include "todo.h"

int install_handler(int signo, void (*fn)(int)) {
    (void)signo;
    (void)fn;
    TODO("implement install_handler");
}

int block_signal(int signo) {
    (void)signo;
    TODO("implement block_signal");
}

int unblock_signal(int signo) {
    (void)signo;
    TODO("implement unblock_signal");
}

int reap_children_count(void) {
    TODO("implement reap_children_count");
}

int selfpipe_init(int pipefd[2]) {
    (void)pipefd;
    TODO("implement selfpipe_init");
}

int selfpipe_wait(int readfd, int timeout_ms) {
    (void)readfd;
    (void)timeout_ms;
    TODO("implement selfpipe_wait");
}
