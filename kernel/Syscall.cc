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
  SYSCALL_DECL(sys_uart_read),
  SYSCALL_DECL(sys_uart_write),
  SYSCALL_DECL(sys_uart_putchar),
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
};



int sys_read(int fd, void* buf, size_t count) {
  // TODO: define stdin...
  if (fd == 0) {
    char* s = reinterpret_cast<char*>(buf);
    for (size_t i = 0; i < count; i++) {
      s[i] = getchar();
    }
    return count;
  }

  SharedPtr<File> file = Task::current()->get_file_by_fd(fd);

  if (!file) {
    printk("sys_read: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::get_instance().read(file, buf, count);
}

int sys_write(int fd, const void* buf, size_t count) {
  // TODO: define stdout and stderr...
  if (fd == 1 || fd == 2) {
    const char* s = reinterpret_cast<const char*>(buf);
    for (size_t i = 0; i < count; i++) {
      putchar(s[i]);
    }
    return count;
  }

  SharedPtr<File> file = Task::current()->get_file_by_fd(fd);

  if (!file) {
    printk("sys_write: fd %d doesn't exist. Is it opened?\n", fd);
    return -1;
  }

  return VFS::get_instance().write(file, buf, count);
}

int sys_open(const char* pathname, int options) {
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

size_t sys_uart_read(char buf[], size_t size) {
  int i = 0;
  while (i < (int) size) {
    auto c = MiniUART::get_instance().getchar();

    if (c == 0x7f) {
      if (i > 0) {
        buf[--i] = 0;
        puts("\b \b", /*newline=*/false);
      }
    } else if (c == '\n') {
      return i;
    } else {
      buf[i++] = c;
    }
  }

  return size;
}

size_t sys_uart_write(const char buf[], size_t size) {
  for (size_t i = 0; i < size; i++) {
    MiniUART::get_instance().putchar(buf[i]);
  }
  return size;
}

void sys_uart_putchar(const char c) {
  MiniUART::get_instance().putchar(c);
}

int sys_fork() {
  return Task::current()->do_fork();
}

int sys_exec(const char* name, const char* const argv[]) {
  return Task::current()->do_exec(name, argv);
}

int sys_wait(int* wstatus) {
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

int sys_signal(int signal, void (*handler)()) {
  return Task::current()->do_signal(signal, handler);
}

int sys_access(const char* pathname, int options) {
  return VFS::get_instance().access(pathname, options);
}

int sys_chdir(const char* pathname) {
  auto& vfs = VFS::get_instance();

  if (VFS::get_instance().access(pathname, 0) == -1) {
    return -1;
  }

  Task::current()->set_cwd_vnode(vfs.resolve_path(pathname)); 
  return 0;
}

int sys_mkdir(const char* pathname) {
  return VFS::get_instance().mkdir(pathname);
}

int sys_rmdir(const char* pathname) {
  return VFS::get_instance().rmdir(pathname);
}

int sys_unlink(const char* pathname) {
  return VFS::get_instance().unlink(pathname);
}

int sys_mount(const char* device_name, const char* mountpoint, const char* fs_name) {
  return VFS::get_instance().mount(device_name, mountpoint, fs_name);
}

int sys_umount(const char* mountpoint) {
  return VFS::get_instance().umount(mountpoint);
}

}  // namespace valkyrie::kernel
