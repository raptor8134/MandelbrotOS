#include <lock.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stddef.h>
#include <stdint.h>

static volatile lock_t vmm_lock = {0};

static uint64_t *kernel_pagemap;

static uint64_t *get_next_level(uint64_t *table, size_t index, uint64_t flags) {
  if (!(table[index] & 1)) {
    table[index] = (uint64_t)pcalloc(1);
    table[index] |= flags;
  }
  return (uint64_t *)((table[index] & ~(0x1ff)) + PHYS_MEM_OFFSET);
}

void vmm_map_page(uint64_t *pagemap, uintptr_t physical_address,
                  uintptr_t virtual_address, uint64_t flags) {
  LOCK(vmm_lock);

  size_t level4 = (size_t)(virtual_address & ((size_t)0x1ff << 39)) >> 39;
  size_t level3 = (size_t)(virtual_address & ((size_t)0x1ff << 30)) >> 30;
  size_t level2 = (size_t)(virtual_address & ((size_t)0x1ff << 21)) >> 21;
  size_t level1 = (size_t)(virtual_address & ((size_t)0x1ff << 12)) >> 12;

  uint64_t *pml3 = get_next_level(pagemap, level4, flags);
  uint64_t *pml2 = get_next_level(pml3, level3, flags);
  uint64_t *pml1 = get_next_level(pml2, level2, flags);

  pml1[level1] = physical_address | flags;

  UNLOCK(vmm_lock);
}

void vmm_unmap_page(uint64_t *pagemap, uintptr_t virtual_address,
                    uint64_t flags) {
  MAKE_LOCK(vmm_lock);

  size_t level4 = (size_t)(virtual_address & ((size_t)0x1ff << 39)) >> 39;
  size_t level3 = (size_t)(virtual_address & ((size_t)0x1ff << 30)) >> 30;
  size_t level2 = (size_t)(virtual_address & ((size_t)0x1ff << 21)) >> 21;
  size_t level1 = (size_t)(virtual_address & ((size_t)0x1ff << 12)) >> 12;

  uint64_t *pml3 = get_next_level(pagemap, level4, flags);
  uint64_t *pml2 = get_next_level(pml3, level3, flags);
  uint64_t *pml1 = get_next_level(pml2, level2, flags);

  pml1[level1] = 0;

  UNLOCK(vmm_lock);
}

void vmm_switch_map_to_kern() {
  asm volatile("mov %0, %%cr3" : : "a"(kernel_pagemap));
}

uint64_t *get_kernel_pagemap() { return kernel_pagemap; }

int init_vmm() {
  kernel_pagemap = (uint64_t *)pcalloc(1);

  for (uintptr_t i = 0; i < 0x80000000; i += PAGE_SIZE)
    vmm_map_page(kernel_pagemap, i, i + KERNEL_MEM_OFFSET, 0b111);

  for (uintptr_t i = 0; i < 0x200000000; i += PAGE_SIZE)
    vmm_map_page(kernel_pagemap, i, i + PHYS_MEM_OFFSET, 0b11);

  vmm_switch_map_to_kern();

  return 0;
}
