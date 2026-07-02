#include "strings.h"

size_t my_strlen(const char *s) {
  /* size_t length = 0; */
  /* while (*(s + length) != '\0') { */
  /*   length++; */
  /* } */
  /* return length; */

  const char *p = s;
  while (*p) {
    p++;
  }
  return (size_t)(p - s);
}

char *my_strcpy(char *dst, const char *src) {
  size_t index = 0;
  while (*(src + index) != '\0') {
    dst[index] = src[index];
    index++;
  }
  dst[index] = '\0';
  return dst;
}

char *my_strncpy(char *dst, const char *src, size_t n) {
  size_t index = 0;
  while (*(src + index) && index < n) {
    dst[index] = src[index];
    index++;
  }
  for (size_t i = index; i < n; i++) {
    dst[i] = '\0';
  }
  return dst;
}

int my_strcmp(const char *a, const char *b) {
  size_t index = 0;
  while (*(a + index) || *(b + index)) {
    if (a[index] < b[index]) {
      return -1;
    }
    if (a[index] > b[index]) {
      return 1;
    }
    index++;
  }
  return 0;
}

char *my_strcat(char *dst, const char *src) {
  char *d = dst;
  while (*d) {
    d++;
  }
  while (*src) {
    *d++ = *src++;
  }
  return dst;
}

char *my_strchr(const char *s, int c) {
  for (;; s++) {
    if (*s == c) {
      return (char *)s;
    } else if (*s == '\0') {
      return NULL;
    }
  }
}

char *my_strstr(const char *haystack, const char *needle) {
  if (*needle == '\0') {
    return (char *)haystack;
  }
  size_t lengthHaystack = my_strlen(haystack);
  size_t lengthNeedle = my_strlen(needle);

  if (lengthHaystack < lengthNeedle) {
    return NULL;
  }

  for (size_t i = 0; i <= lengthHaystack - lengthNeedle; i++) {
    const char *h = haystack + i;
    const char *n = needle;
    while (*h && *n && *h == *n) {
      h++;
      n++;
    }
    if (*n == '\0') {
      return (char *)(haystack + i);
    }
  }

  return NULL;
}

void str_reverse(char *s) {
  size_t length = my_strlen(s);
  if (length < 2) {
    return;
  }
  for (size_t i = 0, j = length - 1; i < j; i++, j--) {
    char temp = s[i];
    s[i] = s[j];
    s[j] = temp;
  }
}

bool is_palindrome(const char *s) {
  size_t length = my_strlen(s);
  if (length < 2) {
    return true;
  }
  for (size_t i = 0, j = length - 1; i < j; i++, j--) {
    if (s[i] != s[j]) {
      return false;
    }
  }
  return true;
}
