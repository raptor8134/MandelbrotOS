#ifndef __SETJMP_H__
#define __SETJMP_H__

#include <stdint.h>

typedef uint32_t __jmp_buf[8];

struct __jmp_buf_tag {
  __jmp_buf __jmp_buf;
  int __mask_was_saved;
};

typedef struct __jmp_buf_tag jmp_buf[1];

int _setjmp(struct __jmp_buf_tag __env[1]);
void _longjmp(struct __jmp_buf_tag __env[1], int __val);

#endif
