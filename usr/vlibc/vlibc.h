// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIBC_H_
#define VALKYRIE_LIBC_H_

#include <types.h>
#include <printf.h>

#define O_CREAT ((1 << 3))

#define assert(pred)                \
  do {                              \
    if (!(pred)) {                  \
      printf("assertion failed\n"); \
      exit(1);                      \
    }                               \
  } while (0)


extern "C" {

// System call wrappers
int read(int fd, void* buf, size_t count);
int write(int fd, const void* buf, size_t count);
int open(const char* pathname, int options);
int close(int fd);
int fork();
int exec(const char* name, const char *const argv[]);
[[noreturn]] void exit(int error_code);
int getpid();
int wait(int* wstatus);
int sched_yield();
long kill(pid_t pid, int signal);
int signal(int signal, void (*handler)());
int access(const char* pathname, int options);
int chdir(const char* pathname);
int mkdir(const char* pathname);
int rmdir(const char* pathname);
int unlink(const char* pathname);
int mount(const char* device_name, const char* mountpoint, const char* fs_name);
int umount(const char* mountpoint);
int getcwd(char* buf);

}

#endif  // VALKYRIE_LIBC_H_
