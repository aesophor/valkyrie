#include "printf.h"

extern "C" void putchar(void*, char);
extern "C" int sys_fork();
extern "C" int sys_exec(const char* name, const char* const argv[]);
extern "C" [[noreturn]] void sys_exit(int error_code);
extern "C" long long int sys_getpid();
extern "C" int sys_wait(int* wstatus);

[[noreturn]] void __libc_start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp]\n\
                bl main\n\
                b sys_exit");
}

void fork_test() {
  printf("Fork Test, pid %d\n", sys_getpid());
  int cnt = 1;
  int ret = 0;

  int wstatus;
  int pid;

  if ((ret = sys_fork()) == 0) { // child
    printf("pid: %d, cnt: %d, ptr: 0x%x\n", sys_getpid(), cnt, &cnt);
    ++cnt;
    sys_fork();
    while (cnt < 5) {
      printf("pid: %d, cnt: %d, ptr: 0x%x\n", sys_getpid(), cnt, &cnt);
      ++cnt;
    }
    printf("child terminating...\n");
  } else {
    printf("parent here, pid %d, child %d\n", sys_getpid(), ret);
    printf("parent terminating...\n");

    pid = sys_wait(&wstatus);
    printf("waited child with pid = %d and error_code = %d\n", pid, wstatus);
  }
  asm volatile("mov %0, sp" : "=r" (cnt));
  asm volatile("mov %0, lr" : "=r" (ret));
  printf("SP = 0x%x, LR = 0x%x\n", cnt, ret);
}


int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  /*
  long long int sp;
  asm volatile("mov %0, sp" : "=r" (sp));
  printf("main(): SP = 0x%x\n", sp);
  */

  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  fork_test();
  return 0;
}

extern "C" void sys_uart_putchar(const char c) {
  asm volatile("mov x8, 2 \n\
                mov x0, %0\n\
                svc #0" :: "r" (c));
}

extern "C" void putchar(void*, char c) {
  sys_uart_putchar(c);
}

extern "C" int sys_fork() {
  asm volatile("mov x8, 3\n\
                svc #0");
}

extern "C" int sys_exec(const char* name, const char* const argv[]) {
  asm volatile("mov x8, 4\n\
                mov x0, %0\n\
                mov x1, %1\n\
                svc #0" :: "r" (name), "r" (argv));
}

extern "C" void sys_exit(int error_code) {
  asm volatile("mov x8, 5\n\
                mov x0, %0\n\
                svc #0" :: "r" (error_code));
}

extern "C" long long int sys_getpid() {
  asm volatile("mov x8, 6\n\
                svc #0");
}

extern "C" int sys_wait(int* wstatus) {
  asm volatile("mov x8, 7\n\
                mov x0, %0\n\
                svc #0" :: "r" (wstatus));
}
