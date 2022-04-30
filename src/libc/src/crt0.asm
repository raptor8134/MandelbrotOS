global _start

extern main
extern __crt_load_environ
extern _exit

_start:
  push rdi
  push rsi
  push rcx
  call __crt_load_environ
  pop rcx
  pop rsi
  pop rdi
  call main
  mov rdi, rax
  call _exit
