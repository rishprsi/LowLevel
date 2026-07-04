#include "funcptr.h"

void int_map(int *arr, size_t n, int (*fn)(int)) {
  for (size_t i = 0; i < n; i++) {
    arr[i] = fn(arr[i]);
  }
}

size_t int_filter(const int *src, size_t n, int *dst, bool (*pred)(int)) {
  size_t count = 0;
  for (size_t i = 0; i < n; i++) {
    if (pred(src[i])) {
      dst[count] = src[i];
      count++;
    }
  }
  return count;
}

int int_reduce(const int *arr, size_t n, int init, int (*op)(int acc, int x)) {
  for (size_t i = 0; i < n; i++) {
    init = op(init, arr[i]);
  }

  return init;
}

void sort_ints(int *arr, size_t n, int (*cmp)(int a, int b)) {
  for (size_t i = 0; i < n - 1; i++) {
    for (size_t j = i + 1; j < n; j++) {
      if (cmp(arr[i], arr[j]) == 1) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
      }
    }
  }
}
