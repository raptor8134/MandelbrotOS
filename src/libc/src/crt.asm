global _start
extern main

extern intsyscall

%define SYSCALL_EXIT 10

_start:
  call main
  mov rdi, SYSCALL_EXIT
  mov rsi, rax
  xor rdx, rdx
  xor rcx, rcx
  xor r8, r8
  xor r9, r9
  call intsyscall
