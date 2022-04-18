#include <stdlib.h>
#include <sys/mandelbrot.h>

void _Exit(int status) { intsyscall(SYSCALL_EXIT, status, 0, 0, 0, 0); }

void abort() {}

int abs(int i) { return ((i < 0) ? (-i) : i); }

double atof(char *arr) {
  double val = 0;
  int afterdot = 0;
  double scale = 1;
  int neg = 0;

  if (*arr == '-') {
    arr++;
    neg = 1;
  }
  while (*arr) {
    if (afterdot) {
      scale = scale / 10;
      val = val + (*arr - '0') * scale;
    } else {
      if (*arr == '.')
        afterdot++;
      else
        val = val * 10.0 + (*arr - '0');
    }
    arr++;
  }
  if (neg)
    return -val;
  else
    return val;
}

int atoi(char *p) {
  int k = 0;
  while (*p) {
    k = (k << 3) + (k << 1) + (*p) - '0';
    p++;
  }
  return k;
}

long int atol(char *p) {
  long int k = 0;
  while (*p) {
    k = (k << 3) + (k << 1) + (*p) - '0';
    p++;
  }
  return k;
}

long long int atoll(char *p) {
  long long int k = 0;
  while (*p) {
    k = (k << 3) + (k << 1) + (*p) - '0';
    p++;
  }
  return k;
}
