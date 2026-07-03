#include "memory.h"
#include "string.h"
#include <stdlib.h>

char *my_strdup(const char *s) {
  char *dup = malloc(sizeof(char *));
  memcpy(dup, s, sizeof(char *));
  return dup;
}

void vec_init(IntVec *v) {
  v->data = NULL;
  v->cap = 0;
  v->len = 0;
}

bool vec_push(IntVec *v, int x) {
  if (v->cap == v->len) {
    const size_t newSize = v->cap == 0 ? 4 : v->cap * 2;
    int *newData = realloc(v->data, sizeof(int) * newSize);
    if (newData == NULL) {
      return false;
    }
    v->data = newData;
    v->cap = newSize;
  }
  v->data[v->len++] = x;
  return true;
}

bool vec_pop(IntVec *v, int *out) {
  if (v->len == 0) {
    return false;
  }
  *out = v->data[--v->len];
  return true;
}

int vec_get(const IntVec *v, size_t i) {
  if (i >= v->len) {
    return -1;
  }
  return v->data[i];
}

void vec_free(IntVec *v) {
  int *newData = realloc(v->data, 0);
  v->data = newData;
  v->len = 0;
  v->cap = 0;
}
