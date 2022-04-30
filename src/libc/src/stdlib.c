#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/liballoc.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <unistd.h>

int liballoc_lock() { return 0; }

int liballoc_unlock() { return 0; }

void *liballoc_alloc_pages(size_t pages) {
  return mmap(NULL, pages * getpagesize(), PROT_READ | PROT_WRITE, MAP_ANON, 0,
              0);
}

int liballoc_free_pages(void *ptr, size_t pages) {
  return munmap(ptr, pages * getpagesize());
}

void *malloc(size_t size) { return liballoc_malloc(size); }

void free(void *ptr) { liballoc_free(ptr); }

void *calloc(size_t size, size_t size2) { return liballoc_calloc(size, size2); }

void *realloc(void *ptr, size_t size) { return liballoc_realloc(ptr, size); }

void _Exit(int status) { intsyscall(SYSCALL_EXIT, status, 0, 0, 0, 0); }

void abort() {}

int abs(int i) { return ((i < 0) ? (-i) : i); }

char *getenv(char *name) {
  size_t l = strlen(name);
  if (environ && !name[l] && l)
    for (char **e = environ; *e; e++)
      if (!strncmp(name, *e, l) && (*e)[l] == '=')
        return *e + strlen(name) + 1;
  return NULL;
}

int setenv(char *name, char *value, int overwrite) {
  size_t l = strlen(name);
  if (environ && !name[l] && l)
    for (char **e = environ; *e; e++)
      if (!strncmp(name, *e, l) && (*e)[l] == '=') {
        if (overwrite) {
          char *new_string = malloc(strlen(name) + strlen(value) + 2);
          strcat(new_string, name);
          strcat(new_string, "=");
          strcat(new_string, value);
          *e = new_string;
        }
        return 0;
      }

  char *new_string = malloc(strlen(name) + strlen(value) + 2);
  strcat(new_string, name);
  strcat(new_string, "=");
  strcat(new_string, value);

  size_t env_len = 0;
  while (environ[env_len])
    env_len++;

  environ = realloc(environ, (env_len + 1) * sizeof(char *));
  environ[env_len] = new_string;

  return 0;
}

int putenv(char *name) {
  size_t l = strchr(name, '=') - name;
  if (environ && !name[l] && l)
    for (char **e = environ; *e; e++)
      if (!strncmp(name, *e, l) && (*e)[l] == '=')
        return *e = name, 0;

  size_t env_len = 0;
  while (environ[env_len])
    env_len++;

  environ = realloc(environ, (env_len + 1) * sizeof(char *));
  environ[env_len] = name;

  return 0;
}

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
