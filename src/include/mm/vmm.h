#ifndef __VMM_H__
#define __VMM_H__

#include <stdint.h>

#define KERNEL_MEM_OFFSET 0xffffffff80000000

void vmm_switch_map_to_kern();
uint64_t *get_kernel_pagemap();
int init_vmm();

#endif
