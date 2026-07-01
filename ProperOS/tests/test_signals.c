#include "ctest.h"
#include "signals.h"

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t usr2_seen = 0;

static void on_usr2(int signo) {
    (void)signo;
    usr2_seen = 1;
}

int main(void) {
    /* install_handler delivers via sigaction */
    {
        CHECK_INT_EQ(install_handler(SIGUSR2, on_usr2), 0);
        usr2_seen = 0;
        raise(SIGUSR2);
        CHECK_INT_EQ((int)usr2_seen, 1);
    }

    /* block_signal holds a signal pending; unblock_signal delivers it */
    {
        usr2_seen = 0;
        CHECK_INT_EQ(block_signal(SIGUSR2), 0);
        raise(SIGUSR2);
        CHECK_INT_EQ((int)usr2_seen, 0); /* blocked -> pending, not delivered */

        sigset_t pending;
        CHECK_INT_EQ(sigpending(&pending), 0);
        CHECK_TRUE(sigismember(&pending, SIGUSR2));

        CHECK_INT_EQ(unblock_signal(SIGUSR2), 0);
        CHECK_INT_EQ((int)usr2_seen, 1); /* delivered during unblock */
    }

    /* SIGCHLD-driven reaping of 3 children */
    {
        CHECK_INT_EQ(reap_children_count(), 3);
        /* everything reaped: no zombies left behind */
        CHECK_INT_EQ((int)waitpid(-1, NULL, WNOHANG), -1);
        CHECK_INT_EQ(errno, ECHILD);
    }

    /* self-pipe trick: signal raised from THIS process */
    {
        int fds[2];
        CHECK_INT_EQ(selfpipe_init(fds), 0);

        raise(SIGUSR1);
        CHECK_INT_EQ(selfpipe_wait(fds[0], 2000), 1);

        /* no signal -> timeout */
        CHECK_INT_EQ(selfpipe_wait(fds[0], 50), 0);

        /* self-pipe trick: signal from a forked child after a delay */
        pid_t pid = fork();
        CHECK_TRUE(pid >= 0);
        if (pid == 0) {
            usleep(100 * 1000); /* 100 ms */
            kill(getppid(), SIGUSR1);
            _exit(0);
        }
        CHECK_INT_EQ(selfpipe_wait(fds[0], 5000), 1);
        CHECK_INT_EQ((int)waitpid(pid, NULL, 0), (int)pid);

        close(fds[0]);
        close(fds[1]);
    }

    CTEST_END();
}
