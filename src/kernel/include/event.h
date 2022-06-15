#ifndef __EVENT_H__
#define __EVENT_H__

#include <lock.h>
#include <stddef.h>

typedef struct event {
  size_t pending;
  lock_t lock;
} event_t;

int event_await(event_t **events, size_t count, int block);
void event_trigger(event_t *event);

#endif
