#include "safety.h"
#include <string.h>

size_t safe_strcpy(char *dst, size_t dstsize, const char *src) {
  size_t srcSize = 0;
  while (*(src + srcSize) != '\0') {
    srcSize++;
  }
  if (dstsize == 0) {
    return srcSize;
  }
  for (size_t i = 0; i < dstsize - 1; i++) {
    if (i < srcSize) {
      dst[i] = src[i];
    } else {
      dst[i] = '\0';
    }
  }
  dst[dstsize - 1] = '\0';
  return srcSize;
}

size_t safe_strcat(char *dst, size_t dstsize, const char *src) {
  size_t srcLen = strlen(src);
  size_t dstLen = strlen(dst);
  for (size_t i = dstLen; i < dstsize - 1; i++) {
    if (i - dstLen < srcLen) {
      dst[i] = src[i - dstLen];
    } else {
      dst[i] = '\0';
    }
  }

  dst[dstsize - 1] = '\0';
  size_t actual = srcLen + dstLen;
  return (actual < dstsize ? actual : dstsize);
}

bool copy_n_safe(uint8_t *dst, size_t dstcap, const uint8_t *src, size_t n) {
  if (n > dstcap) {
    return false;
  }
  for (size_t i = 0; i < n; i++) {
    dst[i] = src[i];
  }
  return true;
}
