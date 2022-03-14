// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Syscall.h>

#include <dev/Console.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

#define SYSCALL_DECL(func) reinterpret_cast<const size_t>(func)

// clang-format off
const size_t __syscall_table[Syscall::__NR_syscall] = {
    SYSCALL_DECL(sys_read),
    SYSCALL_DECL(sys_write),
    SYSCALL_DECL(sys_open),
    SYSCALL_DECL(sys_close),
    SYSCALL_DECL(sys_fork),
    SYSCALL_DECL(sys_exec),
    SYSCALL_DECL(sys_exit),
    SYSCALL_DECL(sys_getpid),
    SYSCALL_DECL(sys_wait),
    SYSCALL_DECL(sys_sched_yield),
    SYSCALL_DECL(sys_kill),
    SYSCALL_DECL(sys_signal),
    SYSCALL_DECL(sys_access),
    SYSCALL_DECL(sys_chdir),
    SYSCALL_DECL(sys_mkdir),
    SYSCALL_DECL(sys_rmdir),
    SYSCALL_DECL(sys_unlink),
    SYSCALL_DECL(sys_mount),
    SYSCALL_DECL(sys_umount),
    SYSCALL_DECL(sys_mknod),
    SYSCALL_DECL(sys_getcwd),
    SYSCALL_DECL(sys_mmap),
    SYSCALL_DECL(sys_munmap),
};
// clang-format on

int sys_read(int fd, void __user *buf, size_t count) {
  buf = Task::current()->v2p(buf);

  // TODO: define stdin...
  if (fd == 0) {
    return Console::the().read(reinterpret_cast<char *>(buf), count);
  }

  SharedPtr<File> file = Task::current()->get_file_by_fd(fd);

  if (!file) {
    printk("sys_read: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::the().read(file, buf, count);
}

int sys_write(int fd, const void __user *buf, size_t count) {
  buf = Task::current()->v2p(buf);

  // TODO: define stdout and stderr...
  if (fd == 1 || fd == 2) {
    return Console::the().write(reinterpret_cast<const char *>(buf), count);
  }

  SharedPtr<File> file = Task::current()->get_file_by_fd(fd);

  if (!file) {
    printk("sys_write: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::the().write(file, buf, count);
}

int sys_open(const char __user *pathname, int options) {
  pathname = Task::current()->v2p(pathname);

  SharedPtr<File> file = VFS::the().open(pathname, options);

  if (!file) {
    if (options & O_CREAT) {
      printk("sys_open: unable to create file %s\n", pathname);
    } else {
      printk("sys_open: file %s doesn't exist\n", pathname);
    }
    return -1;
  }

  return Task::current()->allocate_fd_for_file(file);
}

int sys_close(int fd) {
  SharedPtr<File> file = Task::current()->release_fd_and_get_file(fd);

  if (!file) {
    printk("sys_close: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::the().close(file);
}

int sys_fork() {
  return Task::current()->fork();
}

int sys_exec(const char __user *name, const char __user *argv[]) {
  name = Task::current()->v2p(name);
  argv = Task::current()->v2p(argv);
  return Task::current()->exec(name, argv);
}

int sys_wait(int __user *wstatus) {
  wstatus = Task::current()->v2p(wstatus);
  return Task::current()->wait(wstatus);
}

[[noreturn]] void sys_exit(int error_code) {
  Task::current()->exit(error_code);
}

int sys_getpid() {
  return Task::current()->get_pid();
}

int sys_sched_yield() {
  TaskScheduler::the().schedule();
  return 0;  // always succeeds.
}

long sys_kill(pid_t pid, int signal) {
  return Task::current()->kill(pid, static_cast<Signal>(signal));
}

int sys_signal(int signal, void(__user *handler)()) {
  return Task::current()->signal(signal, handler);
}

int sys_access(const char __user *pathname, int options) {
  pathname = Task::current()->v2p(pathname);
  return VFS::the().access(pathname, options);
}

int sys_chdir(const char __user *pathname) {
  pathname = Task::current()->v2p(pathname);

  auto &vfs = VFS::the();

  if (vfs.access(pathname, 0) == -1) [[unlikely]] {
    return -1;
  }

  SharedPtr<Vnode> vnode = vfs.resolve_path(pathname);

  if (!vnode->is_directory()) [[unlikely]] {
    // TODO: error code
    return -1;
  }

  Task::current()->set_cwd_vnode(move(vnode));
  return 0;
}

int sys_mkdir(const char __user *pathname) {
  pathname = Task::current()->v2p(pathname);
  return VFS::the().mkdir(pathname);
}

int sys_rmdir(const char __user *pathname) {
  pathname = Task::current()->v2p(pathname);
  return VFS::the().rmdir(pathname);
}

int sys_unlink(const char __user *pathname) {
  pathname = Task::current()->v2p(pathname);
  return VFS::the().unlink(pathname);
}

int sys_mount(const char __user *device_name, const char __user *mountpoint,
              const char __user *fs_name) {
  device_name = Task::current()->v2p(device_name);
  mountpoint = Task::current()->v2p(mountpoint);
  fs_name = Task::current()->v2p(fs_name);

  return VFS::the().mount(device_name, mountpoint, fs_name);
}

int sys_umount(const char __user *mountpoint) {
  mountpoint = Task::current()->v2p(mountpoint);
  return VFS::the().umount(mountpoint);
}

int sys_mknod(const char __user *pathname, mode_t mode, dev_t dev) {
  pathname = Task::current()->v2p(pathname);
  return VFS::the().mknod(pathname, mode, dev);
}

int sys_getcwd(char __user *buf) {
  return -1;
}

void __user *sys_mmap(void __user *addr, size_t len, int prot, int flags, int fd,
                      int file_offset) {
  return Task::current()->mmap(addr, len, prot, flags, fd, file_offset);
}

int sys_munmap(void __user *addr, size_t len) {
  return Task::current()->munmap(addr, len);
}

}  // namespace valkyrie::kernel
