#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void __crt_load_environ(int argc, char *argv[], char *env[]) {
  (void)argc;
  (void)argv;
  size_t envc = 0;
  if (env)
    while (env[envc])
      envc++;

  environ = calloc(envc + 1, sizeof(char *));
  for (size_t i = 0; i < envc; i++)
    environ[i] = strdup(env[i]);

  stdin = malloc(sizeof(FILE));
  stdout = malloc(sizeof(FILE));
  stderr = malloc(sizeof(FILE));

  *stdin = (FILE){
    .fd = STDIN_FILENO,
  };
  *stdout = (FILE){
    .fd = STDOUT_FILENO,
  };
  *stderr = (FILE){
    .fd = STDERR_FILENO,
  };
}
