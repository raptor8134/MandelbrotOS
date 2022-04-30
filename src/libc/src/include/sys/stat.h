#ifndef __STAT_H__
#define __STAT_H__

#include <stddef.h>
#include <stdint.h>
#include <time.h>

// TODO/TOFIX: Make this use the correct names and types according to POSIX
typedef struct stat {
  uint64_t device_id;
  uint16_t file_serial;
  uint32_t file_mode;
  uint16_t hardlink_count;
  uint16_t user_id;
  uint16_t group_id;
  uint64_t device_type;
  size_t st_size;

  struct timespec last_access_time;
  struct timespec last_modification_time;
  struct timespec last_status_change_time;
  struct timespec creation_time;

  size_t block_size;
  size_t block_count;
} stat_t;

int fstat(int fd, struct stat *statbuf);

#endif
