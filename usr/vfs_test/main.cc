#include "printf.h"

#define O_CREAT ((1 << 3))

using size_t = unsigned long;

extern "C" void putchar(void*, char);
extern "C" [[noreturn]] void sys_exit(int error_code);
extern "C" int sys_read(int fd, void* buf, size_t count);
extern "C" int sys_write(int fd, const void* buf, size_t count);
extern "C" int sys_open(const char* pathname, int options);
extern "C" int sys_close(int fd);

[[noreturn]] void __libc_start_main() {
  // Prepare argc and argv
  asm volatile("ldp x0, x1, [sp]\n\
                bl main\n\
                b sys_exit");
}

int main(int argc, char **argv) {
  init_printf(nullptr, putchar);

  char buf[32];
  for (int i = 0; i < 32; i++) {
    buf[i] = 0;
  }

  int a = sys_open("hello", O_CREAT);
  int b = sys_open("world", O_CREAT);
  sys_write(a, "Hello ", 6);
  sys_write(b, "World!", 6);
  sys_close(a);
  sys_close(b);
  b = sys_open("hello", 0);
  a = sys_open("world", 0);
  int sz;
  sz = sys_read(b, buf, 100);
  sz += sys_read(a, buf + sz, 100);
  buf[sz] = '\0';
  printf("%s\n", buf); // should be Hello World!
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

extern "C" int sys_read(int fd, void* buf, size_t count) {
  asm volatile("mov x8, #11\n\
                mov x0, %0\n\
                mov x1, %1\n\
                mov x2, %2\n\
                svc #0" :: "r"(fd), "r"(buf), "r"(count));
}

extern "C" int sys_write(int fd, const void* buf, size_t count) {
  asm volatile("mov x8, #12\n\
                mov x0, %0\n\
                mov x1, %1\n\
                mov x2, %2\n\
                svc #0" :: "r"(fd), "r"(buf), "r"(count));
}

extern "C" int sys_open(const char* pathname, int options) {
  asm volatile("mov x8, #13\n\
                mov x0, %0\n\
                mov x1, %1\n\
                svc #0" :: "r"(pathname), "r"(options));
}

extern "C" int sys_close(int fd) {
  asm volatile("mov x8, #14\n\
                mov x0, %0\n\
                svc #0" :: "r"(fd));
}
