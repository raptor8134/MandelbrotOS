#include <cpu_locals.h>
#include <dev/device.h>
#include <drivers/ahci.h>
#include <drivers/apic.h>
#include <drivers/rtc.h>
#include <elf.h>
#include <fb/fb.h>
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
#include <sys/idt.h>
#include <sys/syscall.h>
#include <tasking/scheduler.h>
#include <vec.h>

#define ALIGN_UP(__addr, __align) (((__addr) + (__align)-1) & ~((__align)-1))

#define CURRENT_PAGEMAP get_locals()->current_proc->pagemap
#define CURRENT_PROC get_locals()->current_proc

#define MAX_HEAP_SIZE 0x10000

extern void syscall_isr();

int init_syscalls() {
  idt_set_gate(&idt[0x45], 1, 1, syscall_isr);
  return 0;
}

size_t syscall_open(char *path, int flags, int mode) {
  fs_file_t *file = vfs_open((char *)path);

  if (!file) {
    if (!(flags & O_CREAT))
      return (size_t)-1;

    char *str = strdup(path);
    for (ssize_t i = strlen(str) - 1; i >= 0; i--)
      if (str[i] == '/') {
        str[i] = 0;
        break;
      }

    fs_file_t *parent = vfs_open(str);

    kfree(str);

    if (!parent)
      return -1;

    if (flags & O_RDONLY &&
        vfs_check_can_read(parent, CURRENT_PROC->uid, CURRENT_PROC->gid))
      return -1;
    else if (flags & O_WRONLY &&
             vfs_check_can_write(parent, CURRENT_PROC->uid, CURRENT_PROC->gid))
      return -1;
    else if (flags & O_RDWR && (vfs_check_can_read(parent, CURRENT_PROC->uid,
                                                   CURRENT_PROC->gid) ||
                                vfs_check_can_write(parent, CURRENT_PROC->uid,
                                                    CURRENT_PROC->gid)))
      return -1;

    if (flags & O_EXEC &&
        vfs_check_can_exec(parent, CURRENT_PROC->uid, CURRENT_PROC->gid))
      return -1;

    vfs_close(parent);

    file = vfs_create(path, mode, CURRENT_PROC->uid, CURRENT_PROC->gid);
  }

  if (flags & O_RDONLY &&
      vfs_check_can_read(file, CURRENT_PROC->uid, CURRENT_PROC->gid))
    return -1;
  else if (flags & O_WRONLY &&
           vfs_check_can_write(file, CURRENT_PROC->uid, CURRENT_PROC->gid))
    return -1;
  else if (flags & O_RDWR &&
           (vfs_check_can_read(file, CURRENT_PROC->uid, CURRENT_PROC->gid) ||
            vfs_check_can_write(file, CURRENT_PROC->uid, CURRENT_PROC->gid)))
    return -1;

  if (flags & O_EXEC &&
      vfs_check_can_exec(file, CURRENT_PROC->uid, CURRENT_PROC->gid))
    return -1;

  syscall_file_t *sfile = kmalloc(sizeof(syscall_file_t));
  *sfile = (syscall_file_t){
    .file = file,
    .flags = flags,
  };

  sfile->file->offset = 0;

  size_t i;
  for (i = 0; i < FDS_COUNT; i++)
    if (!CURRENT_PROC->fds[i]) {
      CURRENT_PROC->fds[i] = sfile;
      break;
    }

  return i;
}

void syscall_close(size_t id) {
  syscall_file_t *file = CURRENT_PROC->fds[id];
  if (!file)
    return;
  vfs_close(file->file);
  kfree(file);
  CURRENT_PROC->fds[id] = NULL;
}

ssize_t syscall_read(size_t id, uint8_t *buffer, size_t size) {
  syscall_file_t *file = CURRENT_PROC->fds[id];
  if (!file)
    return 0;
  if (!(file->flags & (O_RDONLY | O_RDWR)))
    return 0;

  if (size + file->file->offset > file->file->length && !ISDEV(file->file) &&
      !ISFIFO(file->file))
    size = file->file->length - file->file->offset;

  ssize_t read = vfs_read(file->file, buffer, file->file->offset, size);

  file->file->offset += (size > 0) ? size : 0;

  return read;
}

ssize_t syscall_write(size_t id, uint8_t *buffer, size_t size) {
  syscall_file_t *file = CURRENT_PROC->fds[id];
  if (!file)
    return 0;
  if (!(file->flags & (O_WRONLY | O_RDWR)))
    return 0;

  if (size + file->file->offset > file->file->length && !ISDEV(file->file) &&
      !ISFIFO(file->file))
    size = file->file->length - file->file->offset;

  ssize_t read = vfs_write(file->file, buffer, file->file->offset, size);

  file->file->offset += (size > 0) ? size : 0;

  return read;
}

int syscall_execve(char *path, char **argv, char **env) {
  fs_file_t *file = vfs_open((char *)path);
  if (vfs_check_can_read(file, CURRENT_PROC->uid, CURRENT_PROC->gid) ||
      vfs_check_can_exec(file, CURRENT_PROC->uid, CURRENT_PROC->gid))
    return -1;
  vfs_close(file);

  size_t argc = 0;
  size_t envc = 0;

  vmm_load_pagemap(&kernel_pagemap);
  proc_t *new_proc =
    sched_new_proc(0, CURRENT_PROC->priority, 1, 0, CURRENT_PROC->parent_proc,
                   CURRENT_PROC->uid, CURRENT_PROC->gid, 0, 0, 0);
  vmm_load_pagemap(CURRENT_PAGEMAP);

  uintptr_t stack_top =
    new_proc->pagemap->ranges.data[0]->phys_addr + STACK_SIZE + PHYS_MEM_OFFSET;
  uint64_t *stack = (size_t *)stack_top;

  if (env)
    for (char **elem = (char **)env; *elem; elem++) {
      stack = (void *)stack - (strlen(*elem) + 1);
      strcpy((char *)stack, *elem);
      envc++;
    }

  if (argv)
    for (char **elem = (char **)argv; *elem; elem++) {
      stack = (void *)stack - (strlen(*elem) + 1);
      strcpy((char *)stack, *elem);
      argc++;
    }

  stack = (void *)stack - ((uintptr_t)stack & 0xf);

  uintptr_t sa = new_proc->regs.rsp;

  if (env) {
    *(--stack) = 0;
    stack -= envc;
    for (size_t i = 0; i < envc; i++) {
      sa -= strlen(env[i]) + 1;
      stack[i] = sa;
    }
  }
  void *envp_addr = stack;

  if (argv) {
    *(--stack) = 0;
    stack -= argc;
    for (size_t i = 0; i < argc; i++) {
      sa -= strlen(argv[i]) + 1;
      stack[i] = sa;
    }
  }
  void *argv_addr = stack;

  new_proc->regs.rdi = argc;
  new_proc->regs.rsi =
    (argv) ? (uintptr_t)new_proc->regs.rsp - (stack_top - (uintptr_t)argv_addr)
           : 0;
  new_proc->regs.rdx =
    (env) ? (uintptr_t)new_proc->regs.rsp - (stack_top - (uintptr_t)envp_addr)
          : 0;
  new_proc->regs.rsp -= stack_top - (uintptr_t)stack;
  new_proc->regs.rsp -= (new_proc->regs.rsp & 8);
  new_proc->pid = get_locals()->current_proc->pid;

  for (size_t i = 0; i < (size_t)FDS_COUNT; i++) {
    syscall_file_t *sfile = kmalloc(sizeof(syscall_file_t));
    if (CURRENT_PROC->fds[i]) {
      *sfile = *CURRENT_PROC->fds[i];
      new_proc->fds[i] = sfile;
    }
  }

  elf_run_binary((char *)path, new_proc, 1);

  sched_destroy(get_locals()->current_proc);
  get_locals()->current_proc = NULL;
  sched_await();

  return -1; // Silly little GCC. Don't you understand that we never get here?
}

void *syscall_mmap(mmap_args_t *args) {
  uintptr_t addr;
  size_t needed_pages = ALIGN_UP(args->length, PAGE_SIZE) / PAGE_SIZE;

  if (args->flags & MAP_FIXED) {
    if ((uintptr_t)args->addr % PAGE_SIZE)
      return (void *)-1;
    addr = (uintptr_t)args->addr;
  } else {
    addr = CURRENT_PROC->mmap_anon;
    CURRENT_PROC->mmap_anon += ((needed_pages + 1) * PAGE_SIZE);
  }

  if (args->flags & MAP_ANON) {
    uintptr_t given_pages = (uintptr_t)pcalloc(needed_pages);

    for (size_t i = 0; i < needed_pages * PAGE_SIZE; i += PAGE_SIZE)
      vmm_map_page(CURRENT_PAGEMAP, given_pages + i, addr + i,
                   (args->prot & PROT_WRITE) ? 0b111 : 0b101);

    mmap_range_t *mmap_range = kmalloc(sizeof(mmap_range_t));
    *mmap_range = (mmap_range_t){
      .file = NULL,
      .flags = args->flags,
      .length = needed_pages * PAGE_SIZE,
      .offset = args->offset,
      .prot = args->prot,
      .phys_addr = given_pages,
      .virt_addr = addr,
    };

    vec_push(&CURRENT_PAGEMAP->ranges, mmap_range);
  } else {
    syscall_file_t *file = CURRENT_PROC->fds[args->fd];
    if (!file)
      return (void *)-1;

    size_t len = args->length;
    size_t off = args->offset;
    size_t prt = args->prot;
    size_t flg = args->flags;

    vmm_load_pagemap(&kernel_pagemap);
    (void)vfs_mmap(file->file, CURRENT_PAGEMAP, file, (void *)addr, len, off,
                   prt, flg);
    vmm_load_pagemap(CURRENT_PAGEMAP);
  }

  return (void *)addr;
}

int syscall_munmap(void *addr, size_t length) {
  for (size_t i = 0; i < (size_t)CURRENT_PAGEMAP->ranges.length; i++) {
    if ((void *)CURRENT_PAGEMAP->ranges.data[i]->virt_addr == addr) {
      vec_remove(&CURRENT_PAGEMAP->ranges, CURRENT_PAGEMAP->ranges.data[i]);
      for (size_t i = 0; i < length; i += PAGE_SIZE)
        vmm_unmap_page(CURRENT_PAGEMAP, (uintptr_t)(addr + i));
      if (CURRENT_PAGEMAP->ranges.data[i]->flags & MAP_ANON)
        pmm_free_pages((void *)CURRENT_PAGEMAP->ranges.data[i]->phys_addr,
                       CURRENT_PAGEMAP->ranges.data[i]->length / PAGE_SIZE);
      return 0;
    }
  }
  return 1;
}

void syscall_stat(char *path, stat_t *stat) {
  fs_file_t *file = vfs_open(path);
  vfs_fstat(file, stat);
  vfs_close(file);
}

void syscall_fstat(size_t id, stat_t *stat) {
  syscall_file_t *file = CURRENT_PROC->fds[id];
  if (!file)
    return;

  vfs_fstat(file->file, stat);
}

size_t syscall_getpid() { return CURRENT_PROC->pid; }

size_t syscall_getppid() {
  return (CURRENT_PROC->parent_proc) ? CURRENT_PROC->parent_proc->pid
                                     : (size_t)-1;
}

void syscall_exit(int code) { sched_current_kill_proc(code); }

size_t syscall_fork(registers_t *regs) {
  vmm_load_pagemap(&kernel_pagemap);
  size_t ret = sched_fork(regs);
  vmm_load_pagemap(CURRENT_PAGEMAP);
  return ret;
}

void syscall_gettimeofday(posix_time_t *time) {
  *time = rtc_mktime(rtc_get_datetime());
}

int syscall_fsync(size_t id) {
  syscall_file_t *file = CURRENT_PROC->fds[id];
  if (!file)
    return 1;

  return 0;
}

uint64_t syscall_ioctl(size_t fd, uint64_t cmd, void *arg) {
  syscall_file_t *file = CURRENT_PROC->fds[fd];
  if (!file)
    return -1;

  return vfs_ioctl(file->file, cmd, arg);
}

size_t syscall_seek(size_t fd, ssize_t offset, int type) {
  syscall_file_t *file = CURRENT_PROC->fds[fd];
  if (!file)
    return -1;

  switch (type) {
    case SEEK_END:
      file->file->offset = file->file->length + offset;
      return file->file->offset;
    case SEEK_CUR:
      file->file->offset += offset;
      return file->file->offset;
    case SEEK_SET:
      file->file->offset = offset;
      return file->file->offset;
  }

  return -1;
}

int syscall_waitpid(ssize_t pid, int *status, int options) {
  return sched_waitpid(pid, status, options);
}

int syscall_access(char *path, int mode) {
  fs_file_t *file = vfs_open(path);

  if (!file)
    return -1;

  if (mode == F_OK) {
    vfs_close(file);
    return 0;
  } else if ((mode & R_OK &&
              vfs_check_can_read(file, CURRENT_PROC->uid, CURRENT_PROC->gid)) ||
             (mode & W_OK && vfs_check_can_write(file, CURRENT_PROC->uid,
                                                 CURRENT_PROC->gid)) ||
             (mode & X_OK &&
              vfs_check_can_exec(file, CURRENT_PROC->uid, CURRENT_PROC->gid))) {

    vfs_close(file);
    return -1;
  }

  return 0;
}

int syscall_pipe(int pipefd[2]) {
  fs_file_t *file =
    vfs_mkfifo("unamed_fifo", 0777, CURRENT_PROC->uid, CURRENT_PROC->gid, 0);
  if (!file)
    return -1;

  syscall_file_t *sfile_r = kmalloc(sizeof(syscall_file_t));
  *sfile_r = (syscall_file_t){
    .file = file,
    .flags = O_RDONLY,
  };

  syscall_file_t *sfile_w = kmalloc(sizeof(syscall_file_t));
  *sfile_w = (syscall_file_t){
    .file = file,
    .flags = O_WRONLY,
  };

  file->offset = 0;

  size_t i;
  for (i = 0; i < FDS_COUNT; i++)
    if (!CURRENT_PROC->fds[i]) {
      CURRENT_PROC->fds[i] = sfile_r;
      break;
    }
  pipefd[0] = i;

  for (i = 0; i < FDS_COUNT; i++)
    if (!CURRENT_PROC->fds[i]) {
      CURRENT_PROC->fds[i] = sfile_w;
      break;
    }
  pipefd[1] = i;

  return 0;
}

int syscall_fcntl(int fd, int cmd, int arg) {
  syscall_file_t *file = CURRENT_PROC->fds[fd];

  switch (cmd) {
    case F_DUPFD:;
      syscall_file_t *new_file = kmalloc(sizeof(syscall_file_t *));
      *new_file = *file;

      size_t i;
      for (i = arg; i < FDS_COUNT; i++)
        if (!CURRENT_PROC->fds[i]) {
          CURRENT_PROC->fds[i] = new_file;
          break;
        }
      return i;
  }

  return -1;
}

uint64_t c_syscall_handler(uint64_t rsp) {
  registers_t *registers = (registers_t *)rsp;

  uint64_t ret = registers->rax;

  switch (registers->rdi) {
    case SYSCALL_OPEN:
      ret = syscall_open((char *)registers->rsi, (int)registers->rdx,
                         (int)registers->rcx);
      break;
    case SYSCALL_CLOSE:
      syscall_close((size_t)registers->rsi);
      break;
    case SYSCALL_READ:
      ret =
        syscall_read(registers->rsi, (uint8_t *)registers->rdx, registers->rcx);
      break;
    case SYSCALL_WRITE:
      ret = syscall_write(registers->rsi, (uint8_t *)registers->rdx,
                          registers->rcx);
      break;
    case SYSCALL_EXEC:
      ret = syscall_execve((char *)registers->rsi, (char **)registers->rdx,
                           (char **)registers->rcx);
      break;
    case SYSCALL_MMAP:
      ret = (uint64_t)syscall_mmap((mmap_args_t *)registers->rsi);
      break;
    case SYSCALL_MUNMAP:
      ret = (uint64_t)syscall_munmap((void *)registers->rsi,
                                     (size_t)registers->rdx);
      break;
    case SYSCALL_STAT:
      syscall_stat((char *)registers->rsi, (stat_t *)registers->rdx);
      break;
    case SYSCALL_FSTAT:
      syscall_fstat((size_t)registers->rsi, (stat_t *)registers->rdx);
      break;
    case SYSCALL_GETPID:
      ret = syscall_getpid();
      break;
    case SYSCALL_GETPPID:
      ret = syscall_getppid();
      break;
    case SYSCALL_EXIT:
      syscall_exit((int)registers->rsi);
      break;
    case SYSCALL_FORK:
      ret = syscall_fork(registers);
      break;
    case SYSCALL_GETTIMEOFDAY:
      syscall_gettimeofday((posix_time_t *)registers->rsi);
      break;
    case SYSCALL_FSYNC:
      syscall_fsync((size_t)registers->rsi);
      break;
    case SYSCALL_IOCTL:
      ret = syscall_ioctl((size_t)registers->rsi, registers->rdx,
                          (void *)registers->rcx);
      break;
    case SYSCALL_SEEK:
      ret =
        syscall_seek((size_t)registers->rsi, registers->rdx, registers->rcx);
      break;
    case SYSCALL_WAITPID:
      ret = syscall_waitpid((ssize_t)registers->rsi, (int *)registers->rdx,
                            registers->rcx);
      break;
    case SYSCALL_ACCESS:
      ret = syscall_access((char *)registers->rsi, registers->rdx);
      break;
    case SYSCALL_PIPE:
      ret = syscall_pipe((int *)registers->rsi);
      break;
    case SYSCALL_FCNTL:
      ret = syscall_fcntl((int)registers->rsi, (int)registers->rdx,
                          (int)registers->rcx);
      break;
    default:
      ret = -1;
      break;
  }

  return ret;
}
