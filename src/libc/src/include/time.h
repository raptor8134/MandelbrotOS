#ifndef __TIME_H__
#define __TIME_H__

#include <stddef.h>

// TODO: Make this use the right types
typedef struct timespec {
  size_t tv_sec;
  size_t tv_nsec;
} timespec_t;

#endif
