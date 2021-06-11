// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Syscall.h>

#include <dev/Console.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/Task.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel {

#define SYSCALL_DECL(func) \
  reinterpret_cast<const size_t>(func)

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
};



int sys_read(int fd, void __user* buf, size_t count) {
  buf = copy_from_user<void*>(buf);

  // TODO: define stdin...
  if (fd == 0) {
    return Console::get_instance().read(reinterpret_cast<char*>(buf), count);
  }

  SharedPtr<File> file = Task::current()->get_file_by_fd(fd);

  if (!file) {
    printk("sys_read: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::get_instance().read(file, buf, count);
}

int sys_write(int fd, const void __user* buf, size_t count) {
  buf = copy_from_user<const void*>(buf);

  // TODO: define stdout and stderr...
  if (fd == 1 || fd == 2) {
    return Console::get_instance().write(reinterpret_cast<const char*>(buf), count);
  }

  SharedPtr<File> file = Task::current()->get_file_by_fd(fd);

  if (!file) {
    printk("sys_write: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::get_instance().write(file, buf, count);
}

int sys_open(const char __user* pathname, int options) {
  pathname = copy_from_user<const char*>(pathname);

  SharedPtr<File> file = VFS::get_instance().open(pathname, options);

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

  return VFS::get_instance().close(file);
}

int sys_fork() {
  return Task::current()->do_fork();
}

int sys_exec(const char __user* name, const char __user* argv[]) {
  name = copy_from_user<const char*>(name);
  argv = copy_from_user<const char**>(argv);
  return Task::current()->do_exec(name, argv);
}

int sys_wait(int __user* wstatus) {
  wstatus = copy_from_user<int*>(wstatus);
  return Task::current()->do_wait(wstatus);
}

[[noreturn]] void sys_exit(int error_code) {
  Task::current()->do_exit(error_code);
}

int sys_getpid() {
  return Task::current()->get_pid();
}

int sys_sched_yield() {
  TaskScheduler::get_instance().schedule();
  return 0;  // always succeeds.
}

long sys_kill(pid_t pid, int signal) {
  return Task::current()->do_kill(pid, static_cast<Signal>(signal));
}

int sys_signal(int signal, void (__user* handler)()) {
  return Task::current()->do_signal(signal, handler);
}

int sys_access(const char __user* pathname, int options) {
  pathname = copy_from_user<const char*>(pathname);
  return VFS::get_instance().access(pathname, options);
}

int sys_chdir(const char __user* pathname) {
  pathname = copy_from_user<const char*>(pathname);

  auto& vfs = VFS::get_instance();

  if (vfs.access(pathname, 0) == -1) {
    return -1;
  }

  Task::current()->set_cwd_vnode(vfs.resolve_path(pathname)); 
  return 0;
}

int sys_mkdir(const char __user* pathname) {
  pathname = copy_from_user<const char*>(pathname);
  return VFS::get_instance().mkdir(pathname);
}

int sys_rmdir(const char __user* pathname) {
  pathname = copy_from_user<const char*>(pathname);
  return VFS::get_instance().rmdir(pathname);
}

int sys_unlink(const char __user* pathname) {
  pathname = copy_from_user<const char*>(pathname);
  return VFS::get_instance().unlink(pathname);
}

int sys_mount(const char __user* device_name,
              const char __user* mountpoint,
              const char __user* fs_name) {
  device_name = copy_from_user<const char*>(device_name);
  mountpoint = copy_from_user<const char*>(mountpoint);
  fs_name = copy_from_user<const char*>(fs_name);

  return VFS::get_instance().mount(device_name, mountpoint, fs_name);
}

int sys_umount(const char __user* mountpoint) {
  mountpoint = copy_from_user<const char*>(mountpoint);
  return VFS::get_instance().umount(mountpoint);
}

int sys_mknod(const char __user* pathname, mode_t mode, dev_t dev) {
  pathname = copy_from_user<const char*>(pathname);
  return VFS::get_instance().mknod(pathname, mode, dev);
}

int sys_getcwd(char __user* buf) {
  return 0;
}

}  // namespace valkyrie::kernel
