// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SYSCALL_H_
#define VALKYRIE_SYSCALL_H_

#include <Types.h>
#include <Utility.h>
#include <dev/Console.h>
#include <mm/UserspaceAccess.h>
#include <proc/TrapFrame.h>

namespace valkyrie::kernel {

enum Syscall {
  SYS_READ,
  SYS_WRITE,
  SYS_OPEN,
  SYS_CLOSE,
  SYS_FORK,
  SYS_EXEC,
  SYS_EXIT,
  SYS_GETPID,
  SYS_WAIT,
  SYS_SCHED_YIELD,
  SYS_KILL,
  SYS_SIGNAL,
  SYS_ACCESS,
  SYS_CHDIR,
  SYS_MKDIR,
  SYS_RMDIR,
  SYS_UNLINK,
  SYS_MOUNT,
  SYS_UMOUNT,
  SYS_MKNOD,
  SYS_GETCWD,
  __NR_syscall
};

// System call table
extern const size_t __syscall_table[Syscall::__NR_syscall];

// Individual system call declaration.
int sys_read(int fd, void __user* buf, size_t count);
int sys_write(int fd, const void __user* buf, size_t count);
int sys_open(const char __user* pathname, int options);
int sys_close(int fd);
int sys_fork();
int sys_exec(const char __user* name, const char __user* argv[]);
[[noreturn]] void sys_exit(int error_code);
int sys_getpid();
int sys_wait(int __user* wstatus);
int sys_sched_yield();
long sys_kill(pid_t pid, int signal);
int sys_signal(int signal, void (__user* handler)());
int sys_access(const char __user* pathname, int options);
int sys_chdir(const char __user* pathname);
int sys_mkdir(const char __user* pathname);
int sys_rmdir(const char __user* pathname);
int sys_unlink(const char __user* pathname);
int sys_mount(const char __user* device_name,
              const char __user* mountpoint,
              const char __user* fs_name);
int sys_umount(const char __user* mountpoint);
int sys_mknod(const char __user* pathname, mode_t mode, dev_t dev);
int sys_getcwd(char __user* buf);


inline bool is_syscall_id_valid(const uint64_t id) {
  return id < Syscall::__NR_syscall;
}

// Indirect system call
//
// A system call is issued using the `svc 0` instruction.
// The system call number is passed via register x8,
// the parameters are stored in x0 ~ x5,
// and the return value will be stored in x0.
template <typename... Args>
size_t do_syscall(const uint64_t id, Args... args) {
  // Check if syscall id is valid.
  if (!is_syscall_id_valid(id)) [[unlikely]] {
    printk("bad system call: (id=0x%x)\n", id);
    return -1;
  }

  // Invoke the specified system call.
  return reinterpret_cast<size_t (*)(Args...)>(__syscall_table[id])(args...);
}

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SYSCALL_H_
