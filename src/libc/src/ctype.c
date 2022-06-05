#include <ctype.h>
#include <sys/mandelbrot.h>

int tolower(int c) {
  if (c >= 'A' && c <= 'Z')
    return c + 32;
  return c;
}

int _tolower(int c) { return tolower(c); }

int toupper(int c) {
  if (c >= 'a' && c <= 'z')
    return c - 32;
  return c;
}

int _toupper(int c) { return toupper(c); }

int isdigit(int c) {
  if (c >= '0' && c <= '9')
    return 1;
  return 0;
}
