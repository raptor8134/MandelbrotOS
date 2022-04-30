#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char *strchr(char *str, int c) {
  for (; *str && *str != c; str++)
    ;
  return (*str) ? str : NULL;
}

size_t strlen(char *s) {
  size_t count = 0;
  while (*s != '\0') {
    count++;
    s++;
  }
  return count;
}

void memcpy(void *dest, void *src, size_t n) {
  for (size_t i = 0; i < n; i++)
    ((char *)dest)[i] = ((char *)src)[i];
}

char *strdup(char *s) {
  char *str = malloc(strlen(s) + 1);
  memcpy(str, s, strlen(s) + 1);
  return str;
}

int strncmp(char *_l, char *_r, size_t n) {
  uint8_t *l = (void *)_l;
  uint8_t *r = (void *)_r;
  if (!n--)
    return 0;
  for (; *l && *r && n && *l == *r; l++, r++, n--)
    ;
  return *l - *r;
}

void strcat(char *dest, char *src) {
  while (*dest)
    dest++;
  while ((*dest++ = *src++))
    ;
}
