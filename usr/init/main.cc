#include "printf.h"

extern "C" void putchar(void*, char);
extern "C" int sys_fork();
extern "C" int sys_exec(const char* name, const char* const argv[]);
extern "C" void sys_exit();
extern "C" long long int sys_getpid();
extern "C" void sys_timer_irq_enable();

int start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp]\n\
                bl main");
}

int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  char fmt[64] = "[init] started... pid: %d\n";
  printf(fmt, sys_getpid());

  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("%s\n", argv[i]);
  }

  const char *fork_argv[] = {"bin/fork_test", 0};
  sys_exec("bin/fork_test", fork_argv);

  sys_exit();
  return 0;
}

extern "C" void sys_uart_putchar(const char c) {
  asm volatile("mov x8, #2 \n\
                mov x0, %0\n\
                svc #0" :: "r" (c));
}

extern "C" void putchar(void*, char c) {
  sys_uart_putchar(c);
}

extern "C" int sys_fork() {
  asm volatile("mov x8, #3\n\
                svc #0");
}

extern "C" int sys_exec(const char* name, const char* const argv[]) {
  asm volatile("mov x8, #4\n\
                mov x0, %0\n\
                mov x1, %1\n\
                svc #0" :: "r" (name), "r" (argv));
}

extern "C" void sys_exit() {
  asm volatile("mov x8, #5\n\
                svc #0");
}

extern "C" long long int sys_getpid() {
  asm volatile("mov x8, #6\n\
                svc #0");
}

extern "C" void sys_timer_irq_enable() {
  asm volatile("mov x8, #7\n\
                svc #0");
}
