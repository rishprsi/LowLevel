#include "procs.h"
#include "todo.h"

int run_command(const char *path, char *const argv[]) {
    (void)path;
    (void)argv;
    TODO("implement run_command");
}

int spawn_capture(const char *path, char *const argv[], char *out,
                  size_t outsz) {
    (void)path;
    (void)argv;
    (void)out;
    (void)outsz;
    TODO("implement spawn_capture");
}
