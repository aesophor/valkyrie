// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_LIBC_H_
#define VALKYRIE_LIBC_H_

#include <printf.h>
#include <types.h>

#define O_CREAT ((1 << 3))

// do_mmap() prots
#define PROT_NONE 0x0  /* Page can not be accessed. */
#define PROT_READ 0x1  /* Page can be read. */
#define PROT_WRITE 0x2 /* Page can be written. */
#define PROT_EXEC 0x4  /* Page can be executed. */

// do_mmap() flags
#define MAP_FIXED 0x10       /* Interpret addr exactly. */
#define MAP_ANONYMOUS 0x20   /* Don't use a file. */
#define MAP_POPULATE 0x08000 /* Populate (prefault) pagetables. */

#define assert(pred)                \
  do {                              \
    if (!(pred)) {                  \
      printf("assertion failed\n"); \
      exit(1);                      \
    }                               \
  } while (0)

extern "C" {

// System call wrappers
int read(int fd, void *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int open(const char *pathname, int options);
int close(int fd);
int fork();
int exec(const char *name, const char *const argv[]);
[[noreturn]] void exit(int error_code);
int getpid();
int wait(int *wstatus);
int sched_yield();
long kill(pid_t pid, int signal);
int signal(int signal, void (*handler)());
int access(const char *pathname, int options);
int chdir(const char *pathname);
int mkdir(const char *pathname);
int rmdir(const char *pathname);
int unlink(const char *pathname);
int mount(const char *device_name, const char *mountpoint, const char *fs_name);
int umount(const char *mountpoint);
int getcwd(char *buf);
void *mmap(void *addr, size_t len, int prot, int flags, int fd, int file_offset);
int munmap(void *addr, size_t len);
}

#endif  // VALKYRIE_LIBC_H_
