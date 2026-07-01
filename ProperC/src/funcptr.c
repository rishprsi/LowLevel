#include "funcptr.h"
#include "todo.h"

void int_map(int *arr, size_t n, int (*fn)(int)) {
    (void)arr;
    (void)n;
    (void)fn;
    TODO("implement int_map");
}

size_t int_filter(const int *src, size_t n, int *dst, bool (*pred)(int)) {
    (void)src;
    (void)n;
    (void)dst;
    (void)pred;
    TODO("implement int_filter");
}

int int_reduce(const int *arr, size_t n, int init, int (*op)(int acc, int x)) {
    (void)arr;
    (void)n;
    (void)init;
    (void)op;
    TODO("implement int_reduce");
}

void sort_ints(int *arr, size_t n, int (*cmp)(int a, int b)) {
    (void)arr;
    (void)n;
    (void)cmp;
    TODO("implement sort_ints");
}
