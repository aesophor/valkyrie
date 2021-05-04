#include "printf.h"

extern "C" void putchar(void*, char);
extern "C" int sys_fork();
extern "C" int sys_exec(const char* name, const char* const argv[]);
extern "C" void sys_exit(int error_code);
extern "C" long long int sys_getpid();
extern "C" int sys_wait(int* wstatus);

[[noreturn]] void __libc_start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp]\n\
                b main");
}

int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  printf("[init] started... pid = %d\n", sys_getpid());
  printf("argc = %d\n", argc);
  printf("argv = 0x%x\n", argv);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] (0x%x) = %s\n", i, argv[i], argv[i]);
  }

  int pid;
  int wstatus;

  switch ((pid = sys_fork())) {
    case -1:  // error
      printf("[init] fork failed\n");
      break;

    case 0: { // child
      sys_exec("bin/vfs_test", nullptr);
      break;
    }

    default:  // parent
      while (true) {
        pid = sys_wait(&wstatus);
        if (pid != -1) {
          printf("[init] reaped zombie: pid = %d with error_code = %d\n", pid, wstatus);
        }
      }
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
