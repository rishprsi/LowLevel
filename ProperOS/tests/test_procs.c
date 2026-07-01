#include "ctest.h"
#include "procs.h"

int main(void) {
    /* run_command: exit statuses */
    {
        char *argv_true[] = {"true", NULL};
        CHECK_INT_EQ(run_command("/usr/bin/true", argv_true), 0);

        char *argv_false[] = {"false", NULL};
        CHECK_INT_EQ(run_command("/usr/bin/false", argv_false), 1);

        /* exec failure inside the child -> 127 */
        char *argv_bad[] = {"nope", NULL};
        CHECK_INT_EQ(run_command("/nonexistent/binary", argv_bad), 127);
    }

    /* spawn_capture: /bin/echo with args */
    {
        char out[128];
        char *argv_echo[] = {"echo", "hello", "capture", NULL};
        int n = spawn_capture("/bin/echo", argv_echo, out, sizeof out);
        CHECK_INT_EQ(n, 14); /* "hello capture\n" */
        CHECK_STR_EQ(out, "hello capture\n");
    }

    /* spawn_capture: empty output */
    {
        char out[16];
        char *argv_true[] = {"true", NULL};
        CHECK_INT_EQ(spawn_capture("/usr/bin/true", argv_true, out, sizeof out), 0);
        CHECK_STR_EQ(out, "");
    }

    /* spawn_capture: child failure -> -1 */
    {
        char out[16];
        char *argv_false[] = {"false", NULL};
        CHECK_INT_EQ(spawn_capture("/usr/bin/false", argv_false, out, sizeof out), -1);
    }

    /* spawn_capture: output longer than the buffer is truncated, still
     * NUL-terminated, and the child is fully drained (no deadlock) */
    {
        char out[8];
        char *argv_echo[] = {"echo", "0123456789abcdef", NULL};
        int n = spawn_capture("/bin/echo", argv_echo, out, sizeof out);
        CHECK_INT_EQ(n, 7);
        CHECK_STR_EQ(out, "0123456");
    }

    CTEST_END();
}
