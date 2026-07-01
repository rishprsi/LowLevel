#ifndef PROPERC_FUNCPTR_H
#define PROPERC_FUNCPTR_H

#include <stdbool.h>
#include <stddef.h>

/* Apply fn to every element of arr[0..n) in place. */
void int_map(int *arr, size_t n, int (*fn)(int));

/*
 * Copy each element of src[0..n) for which pred(element) is true into dst,
 * preserving order. dst must have room for up to n ints. Returns the count
 * written.
 */
size_t int_filter(const int *src, size_t n, int *dst, bool (*pred)(int));

/*
 * Left fold: acc starts at init, then acc = op(acc, arr[i]) for each element.
 * Returns the final accumulator.
 */
int int_reduce(const int *arr, size_t n, int init, int (*op)(int acc, int x));

/*
 * Sort arr[0..n) in ascending order according to cmp, where cmp(a, b) returns
 * a negative / zero / positive value when a should sort before / same as /
 * after b.
 */
void sort_ints(int *arr, size_t n, int (*cmp)(int a, int b));

#endif /* PROPERC_FUNCPTR_H */
