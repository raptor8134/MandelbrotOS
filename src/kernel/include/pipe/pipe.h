#ifndef __PIPE_H__
#define __PIPE_H__

#include <fs/vfs.h>
#include <mm/pmm.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_PIPE_SIZE PAGE_SIZE * 16

typedef struct pipe {
  char *buffer;
  char *buffer_end;
  char *head;
  char *tail;
} pipe_t;

int pipe_init(pipe_t *pipe, size_t capacity);
void pipe_free(pipe_t *pipe);
ssize_t pipe_write(pipe_t *pipe, uint8_t *item, size_t count);
ssize_t pipe_read(pipe_t *pipe, uint8_t *item, size_t count);

#endif
