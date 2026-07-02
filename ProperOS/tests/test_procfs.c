#include "ctest.h"
#include "procfs.h"

#ifdef __linux__
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

int main(void) {
#ifndef __linux__
    fprintf(stderr, "SKIP: procfs is Linux-only\n");
    CTEST_END(); /* 0 checks, exit 0 */
#else
    /* count_maps_regions: any live process has several mappings */
    SECTION("count_maps_regions");
    int n = count_maps_regions();
    CHECK_TRUE(n > 0);

    /* mapping a new region should not DECREASE the count */
    void *extra = malloc(4 * 1024 * 1024); /* likely a fresh mmap region */
    CHECK_PTR_NONNULL(extra);
    ((volatile char *)extra)[0] = 1;
    CHECK_TRUE(count_maps_regions() >= n);
    free(extra);

    /* read_status_field: Pid must equal getpid() */
    SECTION("read_status_field Pid");
    char buf[128];
    CHECK_INT_EQ(read_status_field("Pid", buf, sizeof buf), 0);
    CHECK_INT_EQ(atoi(buf), (int)getpid());

    /* Name: value is non-empty and has no trailing newline */
    SECTION("read_status_field Name");
    CHECK_INT_EQ(read_status_field("Name", buf, sizeof buf), 0);
    CHECK_TRUE(strlen(buf) > 0);
    CHECK_TRUE(buf[strlen(buf) - 1] != '\n');

    /* unknown field -> -1 */
    SECTION("read_status_field unknown");
    CHECK_INT_EQ(read_status_field("NoSuchFieldXYZ", buf, sizeof buf), -1);

    CTEST_END();
#endif
}
