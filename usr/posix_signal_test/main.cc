#include "printf.h"

extern "C" void putchar(void*, char);
extern "C" [[noreturn]] void sys_exit(int error_code);
extern "C" long long int sys_getpid();
extern "C" int sys_wait(int* wstatus);

[[noreturn]] void __libc_start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp]\n\
                bl main\n\
                b sys_exit");
}

void posix_signal_test() {

}


int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  posix_signal_test();
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

extern "C" void sys_exit(int error_code) {
  asm volatile("mov x8, 5\n\
                mov x0, %0\n\
                svc #0" :: "r" (error_code));
}

extern "C" long long int sys_getpid() {
  asm volatile("mov x8, 6\n\
                svc #0");
}
