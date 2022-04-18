#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <fs/vfs.h>
#include <lock.h>
#include <mm/vmm.h>
#include <registers.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/syscall.h>

#define SCHED_PRIORITY_LEVELS 16
#define DEFAULT_TIMESLICE 5000
#define DEFAULT_WAIT_TIMESLICE 20000

typedef struct proc {
  int user;
  int blocked;
  int enqueued;
  pagemap_t *pagemap;
  size_t pid;
  struct proc *parent_proc;
  uintptr_t mmap_anon;
  size_t last_fd;
  vec_t(syscall_file_t *) fds;
  vec_t(struct proc *) children;
  lock_t lock;
  uint8_t priority;
  uint8_t fpu_storage[512] __attribute__((aligned(16)));
  uintptr_t kernel_stack;
  registers_t regs;
} proc_t;

void scheduler_init(uintptr_t addr);
void sched_await();
void sched_current_kill_proc();
size_t sched_fork(registers_t *regs);
void sched_enqueue_proc(proc_t *proc);
proc_t *sched_new_proc(uintptr_t addr, uint8_t priority, int user,
                       int auto_enqueue, proc_t *parent_proc, uint64_t arg1,
                       uint64_t arg2, uint64_t arg3);

#endif
