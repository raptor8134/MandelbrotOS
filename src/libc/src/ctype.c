#include <ctype.h>
#include <sys/mandelbrot.h>

int isdigit(int c) { return c >= '0' && c <= '9'; }

int isspace(int c) {
  return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
         c == '\v';
}

int islower(int c) { return c >= 'a' && c <= 'z'; }

int isupper(int c) { return c >= 'A' && c <= 'Z'; }

int isxdigit(int c) {
  return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

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

int isalnum(int c) { return isdigit(c) || isalpha(c); }

int isalpha(int c) { return isupper(c) || islower(c); }

int isblank(int c) { return c == ' ' || c == '\t'; }

int iscntrl(int c) { return (c >= 0 && c < ' ') || c == '\b'; }
