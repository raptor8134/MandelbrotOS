#include <elf.h>
#include <fb/fb.h>
#include <fs/vfs.h>
#include <mm/kheap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <printf.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <tasking/scheduler.h>

#define ELF_RELOCATEABLE 1
#define ELF_EXECUTABLE 2

#define ELF_HEAD_LOAD 1

#define PF_X 1
#define PF_W 2
#define PF_R 4

#define ROUND_UP(A, B)                                                         \
  ({                                                                           \
    typeof(A) _a_ = A;                                                         \
    typeof(B) _b_ = B;                                                         \
    (_a_ + (_b_ - 1)) / _b_;                                                   \
  })

char elf_ident[4] = {0x7f, 'E', 'L', 'F'};

uint8_t elf_run_binary(char *path, proc_t *proc, int auto_enqueue) {
  fs_file_t *file = vfs_open(path);
  uint8_t *buffer = kmalloc(file->length);
  vfs_read(file, buffer, 0, file->length);

  elf_header_t *header = (elf_header_t *)buffer;
  if (header->type != ELF_EXECUTABLE)
    return 1;
  if (memcmp((void *)header->identifier, elf_ident, 4))
    return 1;

  elf_header_t *elf_header = (elf_header_t *)buffer;
  elf_prog_header_t *prog_header = (void *)(buffer + elf_header->prog_head_off);

  for (size_t i = 0; i < elf_header->prog_head_count; i++) {
    if (prog_header[i].type == ELF_HEAD_LOAD) {
      void *addr = pcalloc(ROUND_UP(
        (prog_header[i].virt_addr & (PAGE_SIZE - 1)) + prog_header[i].mem_size,
        PAGE_SIZE));

      for (size_t j = 0;
           j < ROUND_UP((prog_header[i].virt_addr & (PAGE_SIZE - 1)) +
                          prog_header[i].mem_size,
                        PAGE_SIZE) *
                 PAGE_SIZE;
           j += PAGE_SIZE)
        vmm_map_page(proc->pagemap, (uintptr_t)addr + j,
                     prog_header->virt_addr + j,
                     (prog_header[i].flags & PF_W) ? 0b111 : 0b101);

      mmap_range_t *mmap_range = kmalloc(sizeof(mmap_range_t));
      *mmap_range = (mmap_range_t){
        .file = NULL,
        .flags = MAP_FIXED | MAP_ANON,
        .length = ROUND_UP((prog_header[i].virt_addr & (PAGE_SIZE - 1)) +
                             prog_header[i].mem_size,
                           PAGE_SIZE) *
                  PAGE_SIZE,
        .offset = 0,
        .prot = PROT_READ | PROT_EXEC | (prog_header[i].flags & PF_W)
                  ? PROT_WRITE
                  : 0,
        .phys_addr = (uintptr_t)addr,
        .virt_addr = prog_header[i].virt_addr,
      };

      vec_push(&proc->pagemap->ranges, mmap_range);

      memcpy(((char *)((uintptr_t)addr + PHYS_MEM_OFFSET)) +
               (prog_header[i].virt_addr & (PAGE_SIZE - 1)),
             buffer + prog_header[i].offset, prog_header[i].file_size);
    }
  }

  proc->regs.rip = header->entry;

  if (auto_enqueue)
    sched_enqueue_proc(proc);

  return 0;
}
