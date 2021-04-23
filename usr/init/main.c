#include "printf.h"

extern "C" void putchar(void*, char);
extern "C" void sys_exit();
extern "C" long long int sys_getpid();

int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  printf("damn\n");

  char fmt[64] = "[init] started... pid: %d\n";
  printf(fmt, sys_getpid());

  /*
  printf("Argv Test, pid %d\n", sys_getpid());
  for (int i = 0; i < argc; ++i) {
    puts(argv[i]);
  }
  char *fork_argv[] = {"fork_test", 0};
  sys_exec("fork_test", fork_argv);
  */

  sys_exit();
  return 0;
}

extern "C" void sys_uart_putchar(const char c) {
  asm volatile("mov x8, 2 \n\
                mov x0, %0\n\
                svc #0" :: "r" (c));
}

extern "C" void putchar(void*, char c) {
  asm volatile("mov x8, 2 \n\
                mov x0, %0\n\
                svc #0" :: "r" (c));

//  sys_uart_putchar(c);
}

extern "C" void sys_exit() {
  asm volatile("mov x8, 5\n\
                svc #0");
}

extern "C" long long int sys_getpid() {
  asm volatile("mov x8, 6\n\
                svc #0");
}
