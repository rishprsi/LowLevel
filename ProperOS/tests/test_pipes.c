#include "ctest.h"
#include "pipes.h"

#include <string.h>

int main(void) {
    /* trim_ws */
    {
        SECTION("trim_ws");
        char a[] = "       2\n";
        CHECK_STR_EQ(trim_ws(a), "2");
        char b[] = "no-trim";
        CHECK_STR_EQ(trim_ws(b), "no-trim");
        char c[] = "  \t \n ";
        CHECK_STR_EQ(trim_ws(c), "");
        char d[] = "";
        CHECK_STR_EQ(trim_ws(d), "");
        char e[] = " middle stays ";
        CHECK_STR_EQ(trim_ws(e), "middle stays");
    }

    /* pipeline2: echo "hello world" | wc -w  ->  2 */
    {
        SECTION("pipeline2");
        char *cmd1[] = {"/bin/echo", "hello world", NULL};
        char *cmd2[] = {"/usr/bin/wc", "-w", NULL};
        char out[64];
        CHECK_INT_EQ(pipeline2(cmd1, cmd2, out, sizeof out), 0);
        CHECK_STR_EQ(trim_ws(out), "2");
    }

    /* pipeline2: echo through cat is identity */
    {
        SECTION("pipeline2 identity");
        char *cmd1[] = {"/bin/echo", "pass-through", NULL};
        char *cmd2[] = {"/bin/cat", NULL};
        char out[64];
        CHECK_INT_EQ(pipeline2(cmd1, cmd2, out, sizeof out), 0);
        CHECK_STR_EQ(out, "pass-through\n");
    }

    /* pipeline2: a failing stage makes the whole pipeline fail */
    {
        SECTION("pipeline2 failing stage");
        char *cmd1[] = {"/bin/echo", "x", NULL};
        char *cmd2[] = {"/usr/bin/false", NULL};
        char out[64];
        CHECK_INT_EQ(pipeline2(cmd1, cmd2, out, sizeof out), -1);
    }

    /* pipeline3: echo "a b c d" | cat | wc -w  ->  4 */
    {
        SECTION("pipeline3");
        char *cmd1[] = {"/bin/echo", "a b c d", NULL};
        char *cmd2[] = {"/bin/cat", NULL};
        char *cmd3[] = {"/usr/bin/wc", "-w", NULL};
        char out[64];
        CHECK_INT_EQ(pipeline3(cmd1, cmd2, cmd3, out, sizeof out), 0);
        CHECK_STR_EQ(trim_ws(out), "4");
    }

    CTEST_END();
}
