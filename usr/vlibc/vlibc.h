// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIBC_H_
#define VALKYRIE_LIBC_H_

#include <types.h>
#include <printf.h>

extern "C" {

// System call wrappers
int read(int fd, void* buf, size_t count);
int write(int fd, const void* buf, size_t count);
int open(const char* pathname, int options);
int close(int fd);
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
void uart_putchar(const char c);
int fork();
int exec(const char* name, const char *const argv[]);
[[noreturn]] void exit(int error_code);
int getpid();
int wait(int* wstatus);
int sched_yield();
long kill(pid_t pid, int signal);
int signal(int signal, void (*handler)());


inline void __libc_putchar(void*, const char c) {
  uart_putchar(c);
}

}

#endif  // VALKYRIE_LIBC_H_
