#include "printf.h"

extern "C" void putchar(void*, char);
extern "C" int sys_fork();
extern "C" int sys_exec(const char* name, const char* const argv[]);
extern "C" void sys_exit(int error_code);
extern "C" long long int sys_getpid();
extern "C" int sys_wait(int* wstatus);

int start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp]\n\
                bl main");
}

int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  char fmt[64] = "[init] started... pid: %d\n";
  printf(fmt, sys_getpid());

  int pid;
  int wstatus;

  switch ((pid = sys_fork())) {
    case -1:  // error
      printf("[init] fork failed\n");
      break;

    case 0: { // child
      const char* fork_argv[] = {"bin/fork_test", nullptr};
      sys_exec("bin/fork_test", fork_argv);
      break;
    }

    default:  // parent
      sys_wait(&wstatus);
      while (1);
      break;
  }

  sys_exit(0);
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

extern "C" void sys_exit(int error_code) {
  asm volatile("mov x8, #5\n\
                mov x0, %0\n\
                svc #0" :: "r" (error_code));
}

extern "C" long long int sys_getpid() {
  asm volatile("mov x8, #6\n\
                svc #0");
}

extern "C" int sys_wait(int* wstatus) {
  asm volatile("mov x8, 7\n\
                mov x0, %0\n\
                svc #0" :: "r" (wstatus));
}
