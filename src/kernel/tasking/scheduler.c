#include <boot/stivale2.h>
#include <cpu_locals.h>
#include <drivers/apic.h>
#include <drivers/pcspkr.h>
#include <drivers/pit.h>
#include <drivers/serial.h>
#include <fs/vfs.h>
#include <lock.h>
#include <mm/kheap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <printf.h>
#include <registers.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/gdt.h>
#include <sys/irq.h>
#include <sys/syscall.h>
#include <tasking/scheduler.h>
#include <vec.h>

#define STACK_SIZE 0x20000
#define VIRTUAL_STACK_ADDR 0x70000000000

extern void switch_and_run_stack(uintptr_t stack);

static lock_t sched_lock = {0};
static int sched_started = 0;
static size_t current_pid = 0;

vec_t(proc_t *) procs[SCHED_PRIORITY_LEVELS];

void sched_await() {
  while (!sched_started)
    ;

  asm volatile("cli");

  lapic_timer_oneshot(SCHEDULE_REG, DEFAULT_WAIT_TIMESLICE);

  asm volatile("sti\n"
               "1:\n"
               "hlt\n"
               "jmp 1b\n");
}

void sched_enqueue_proc(proc_t *proc) {
  if (proc->enqueued)
    return;

  LOCK(sched_lock);

  vec_push(&procs[proc->priority], proc);
  proc->enqueued = 1;

  UNLOCK(sched_lock);
}

proc_t *sched_new_proc(uintptr_t addr, uint8_t priority, int user,
                       int auto_enqueue, proc_t *parent_proc, uint64_t arg1,
                       uint64_t arg2, uint64_t arg3) {
  proc_t *new_proc = kmalloc(sizeof(proc_t));

  *new_proc = (proc_t){
    .pid = current_pid++,
    .parent_proc = parent_proc,
    .enqueued = 0,
    .blocked = 0,
    .regs =
      (registers_t){
        .cs = (user) ? GDT_SEG_UCODE : GDT_SEG_KCODE,
        .ss = (user) ? GDT_SEG_UDATA : GDT_SEG_KDATA,
        .rip = (uint64_t)addr,
        .rflags = 0x202,
        .rax = 0,
        .rdi = arg1,
        .rsi = arg2,
        .rdx = arg3,
      },
    .pagemap = (user) ? vmm_create_new_pagemap() : &kernel_pagemap,
    .mmap_anon = MMAP_ANON_START_ADDR,
    .user = user,
    .priority = priority,
    .last_fd = 0,
    .fpu_storage = {0},
  };

  if (new_proc->parent_proc)
    vec_push(&new_proc->parent_proc->children, new_proc);

  new_proc->children.data = kmalloc(sizeof(proc_t *));
  new_proc->fds.data = kmalloc(sizeof(syscall_file_t *));

  asm volatile("fxsave %0" : "+m"(new_proc->fpu_storage) : : "memory");

  if (user) {
    uintptr_t user_stack = (uint64_t)pcalloc(STACK_SIZE / PAGE_SIZE);
    uintptr_t kernel_stack = (uint64_t)pcalloc(STACK_SIZE / PAGE_SIZE);

    for (size_t i = 0; i < STACK_SIZE; i += PAGE_SIZE)
      vmm_map_page(new_proc->pagemap, user_stack + i, VIRTUAL_STACK_ADDR + i,
                   0b111);

    new_proc->regs.rsp = VIRTUAL_STACK_ADDR + STACK_SIZE;
    new_proc->kernel_stack = kernel_stack + PHYS_MEM_OFFSET + STACK_SIZE;

    mmap_range_t *mmap_range = kmalloc(sizeof(mmap_range_t));
    *mmap_range = (mmap_range_t){
      .file = NULL,
      .flags = MAP_FIXED | MAP_ANON,
      .length = STACK_SIZE,
      .offset = 0,
      .prot = PROT_READ | PROT_WRITE | PROT_EXEC,
      .phys_addr = user_stack,
      .virt_addr = VIRTUAL_STACK_ADDR,
    };

    vec_push(&new_proc->pagemap->ranges, mmap_range);
  } else
    new_proc->regs.rsp = new_proc->kernel_stack =
      (uintptr_t)pcalloc(STACK_SIZE / PAGE_SIZE) + PHYS_MEM_OFFSET + STACK_SIZE;

  if (auto_enqueue)
    sched_enqueue_proc(new_proc);

  return new_proc;
}

size_t sched_fork(registers_t *regs) {
  proc_t *orig_proc = get_locals()->current_proc;

  proc_t *new_proc = kmalloc(sizeof(proc_t));

  *new_proc = (proc_t){
    .user = orig_proc->user,
    .blocked = 0,
    .regs = *regs,
    .priority = orig_proc->priority,
    .pid = current_pid++,
    .parent_proc = orig_proc,
    .pagemap = vmm_fork_pagemap(orig_proc->pagemap),
    .enqueued = 0,
    .last_fd = orig_proc->last_fd,
    .mmap_anon = orig_proc->mmap_anon,
    .kernel_stack =
      (uintptr_t)pcalloc(STACK_SIZE / PAGE_SIZE) + STACK_SIZE + PHYS_MEM_OFFSET,
  };

  new_proc->fds.data = kmalloc(sizeof(syscall_file_t *));
  new_proc->children.data = kmalloc(sizeof(proc_t *));

  for (size_t i = 0; i < (size_t)orig_proc->fds.length; i++) {
    syscall_file_t *sfile = kmalloc(sizeof(syscall_file_t));
    *sfile = *orig_proc->fds.data[i];
    vec_push(&new_proc->fds, sfile);
  }

  new_proc->regs.rax = 0;

  memcpy(new_proc->fpu_storage, orig_proc->fpu_storage, 512);
  memcpy((void *)new_proc->kernel_stack - STACK_SIZE,
         (void *)orig_proc->kernel_stack - STACK_SIZE, STACK_SIZE);

  vec_push(&orig_proc->children, new_proc);

  sched_enqueue_proc(new_proc);

  return new_proc->pid;
}

void sched_destroy(proc_t *proc) {
  LOCK(sched_lock);

  vec_remove(&procs[proc->priority], proc);
  vec_remove(&proc->parent_proc->children, proc);

  for (size_t i = 0; i < (size_t)proc->fds.length; i++)
    vfs_close(proc->fds.data[i]->file);
  for (size_t i = 0; i < (size_t)proc->children.length; i++)
    sched_destroy(proc->children.data[i]);

  pmm_free_pages((void *)proc->kernel_stack, STACK_SIZE / PAGE_SIZE);
  if (proc->user)
    pmm_free_pages((void *)proc->regs.rsp, STACK_SIZE / PAGE_SIZE);

  kfree(proc);

  UNLOCK(sched_lock);
}

void sched_current_kill_proc() {
  proc_t *current_proc = get_locals()->current_proc;
  get_locals()->current_proc = NULL;
  sched_destroy(current_proc);
  sched_await();
}

size_t sched_get_next_proc(size_t orig_i, uint8_t priority) {
  size_t index = orig_i + 1;

  while (1) {
    if (index >= (size_t)procs[priority].length)
      index = 0;

    proc_t *proc = LOCKED_READ(procs[priority].data[index]);

    if (LOCK_ACQUIRE(proc->lock) || proc == get_locals()->current_proc)
      return index;

    if (index == orig_i)
      break;

    index++;
  }

  return -1;
}

void schedule(uint64_t rsp) {
  lapic_timer_stop();

  if (!LOCK_ACQUIRE(sched_lock)) {
    lapic_eoi();
    lapic_timer_oneshot(SCHEDULE_REG, DEFAULT_WAIT_TIMESLICE);
    switch_and_run_stack((uintptr_t)rsp);
  }

  cpu_locals_t *locals = get_locals();
  proc_t *current_proc = locals->current_proc;
  size_t old_index = locals->last_run_thread_index[locals->current_priority];

  do {
    locals->current_priority++;
    if (locals->current_priority > locals->current_priority_peg) {
      locals->current_priority = 0;
      locals->current_priority_peg++;
      if (locals->current_priority_peg == SCHED_PRIORITY_LEVELS)
        locals->current_priority_peg = 0;
    }
  } while (!procs[locals->current_priority].length);

  size_t new_index =
    sched_get_next_proc(locals->last_run_thread_index[locals->current_priority],
                        locals->current_priority);

  if (current_proc) {
    if (new_index == old_index) {
      UNLOCK(sched_lock);
      lapic_eoi();
      lapic_timer_oneshot(SCHEDULE_REG, DEFAULT_TIMESLICE);
      switch_and_run_stack((uintptr_t)rsp);
    }

    vmm_load_pagemap(&kernel_pagemap);

    asm volatile("fxsave %0" : "+m"(current_proc->fpu_storage) : : "memory");
    current_proc->regs = *((registers_t *)rsp);
    UNLOCK(current_proc->lock);

    vmm_load_pagemap(current_proc->pagemap);
  }

  if (new_index == (size_t)-1) {
    lapic_eoi();
    UNLOCK(sched_lock);
    sched_await();
  }

  current_proc = LOCKED_READ(procs[locals->current_priority].data[new_index]);
  locals->current_proc = current_proc;
  locals->last_run_thread_index[locals->current_priority] = new_index;

  asm volatile("fxrstor %0" : : "m"(current_proc->fpu_storage) : "memory");

  locals->tss.rsp[0] = current_proc->kernel_stack;

  lapic_eoi();
  lapic_timer_oneshot(SCHEDULE_REG, DEFAULT_TIMESLICE);

  vmm_load_pagemap(current_proc->pagemap);

  UNLOCK(sched_lock);

  switch_and_run_stack((uintptr_t)&current_proc->regs);
}

void scheduler_init(uintptr_t addr) {
  for (size_t i = 0; i < SCHED_PRIORITY_LEVELS; i++)
    procs[i].data = kmalloc(sizeof(proc_t *));

  sched_new_proc(addr, 0, 0, 1, NULL, 0, 0, 0);

  LOCKED_WRITE(sched_started, 1);
  sched_await();
}
