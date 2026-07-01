#include "pointers.h"
#include <stdio.h>

void swap_int(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void reverse_array(int *arr, size_t n) {
  if (n == 0) {
    return;
  }

  for (size_t i = 0, j = n - 1; i < j; i++, j--) {
    int temp = *(arr + i);
    *(arr + i) = *(arr + j);
    *(arr + j) = temp;
  }
}

void *my_memcpy(void *dst, const void *src, size_t n) {
  /* TODO("implement my_memcpy"); */
  const unsigned char *s = src;
  unsigned char *d = dst;
  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }
  return d;
}

void *my_memmove(void *dst, const void *src, size_t n) {
  /* TODO("implement my_memmove"); */
  unsigned char *d = dst;
  const unsigned char *s = src;
  if (d < s) {
    for (size_t i = 0; i < n; i++) {
      d[i] = s[i];
    }
  } else if (d > s) {
    for (size_t i = n; i > 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }
  return d;
}

void rotate_left(int *arr, size_t n, size_t k) {
  /* TODO("implement rotate_left"); */
  k = k % n;
  if (k == 0 || n == 0) {
    return;
  }
  reverse_array(arr, k);
  reverse_array(arr + k, n - k);
  reverse_array(arr, n);
}
