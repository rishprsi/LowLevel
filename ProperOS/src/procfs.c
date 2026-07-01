#include "procfs.h"

#ifdef __linux__

/* Linux: implement for real. The TODOs below are hard compile errors. */
#include "todo.h"

int count_maps_regions(void) {
    TODO("implement count_maps_regions");
}

int read_status_field(const char *field, char *out, size_t outsz) {
    (void)field;
    (void)out;
    (void)outsz;
    TODO("implement read_status_field");
}

#else /* !__linux__ */

/* No /proc here: compile-clean stubs so macOS builds pass; the test skips. */

int count_maps_regions(void) {
    return -1;
}

int read_status_field(const char *field, char *out, size_t outsz) {
    (void)field;
    (void)out;
    (void)outsz;
    return -1;
}

#endif /* __linux__ */
