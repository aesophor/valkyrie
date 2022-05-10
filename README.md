<div align="center">

<h3>VALKYRIE</h3>
<p>A UNIX-like hobby kernel built from scratch in C++20</p>

<img src="/Documentation/cover.png">
</div>

## Kernel Features

* AArch64 kernel with (user & kernel) preemptive multi-threading
* Copy-on-write `fork()`
* Virtual memory
* Virtual filesystem (VFS)
* FAT32 filesystem (supports long filenames)
* /dev, /proc, /tmp filesystem
* [Self-made C++ standard library](https://github.com/aesophor/valkyrie/tree/master/include/lib)

## Build and Run

#### Build requirements

Platform: macOS or (preferably Arch) Linux running on Intel CPU

* GNU make
* aarch64 cross compiler toolchain
* qemu-system-aarch64

#### Installing ARMv8 cross compiler toolchain and QEMU

```sh
# macOS (Intel)
brew tap messense/macos-cross-toolchains
brew install aarch64-unknown-linux-gnu qemu

# Arch Linux (Intel)
sudo pacman -S aarch64-linux-gnu-gcc aarch64-linux-gnu-gdb qemu-arch-extra
```

#### Building and running valkyrie

```
git clone https://github.com/aesophor/valkyrie
cd valkyrie
make && make run
```

## Syscalls

```cpp
int sys_read(int fd, void __user *buf, size_t count);
int sys_write(int fd, const void __user *buf, size_t count);
int sys_open(const char __user *pathname, int options);
int sys_close(int fd);
int sys_fork();
int sys_exec(const char __user *name, const char __user *argv[]);
[[noreturn]] void sys_exit(int error_code);
int sys_getpid();
int sys_wait(int __user *wstatus);
int sys_sched_yield();
long sys_kill(pid_t pid, int signal);
int sys_signal(int signal, void(__user *handler)());
int sys_access(const char __user *pathname, int options);
int sys_chdir(const char __user *pathname);
int sys_mkdir(const char __user *pathname);
int sys_rmdir(const char __user *pathname);
int sys_unlink(const char __user *pathname);
int sys_mount(const char __user *device_name, const char __user *mountpoint,
              const char __user *fs_name);
int sys_umount(const char __user *mountpoint);
int sys_mknod(const char __user *pathname, mode_t mode, dev_t dev);
int sys_getcwd(char __user *buf);
void __user *sys_mmap(void __user *addr, size_t len, int prot, int flags, int fd,
                      int file_offset);
int sys_munmap(void __user *addr, size_t len);
```

## User Programs

* init
* login
* sh
* ls
* cat
* mkdir
* touch
* fork_test
* page_fault_test
* procfs_test
* vfs_test_dev
* vfs_test_mnt
* vfs_test_orw 
* mmap_illegal_read
* mmap_illegal_write

## References

* [國立陽明交通大學 資訊科學與工程研究所, Operating System Capstone, Spring 2021](https://grasslab.github.io/NYCU_Operating_System_Capstone/)
* [Linux](https://github.com/torvalds/linux)
* [SerenityOS](https://github.com/SerenityOS/serenity)
* [OS67](https://github.com/SilverRainZ/OS67)

## License

[GNU General Public License v3](https://github.com/aesophor/valkyrie/blob/309551004/LICENSE)
