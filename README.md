<div align="center">

<h3>VALKYRIE</h3>
<p>A UNIX-like toy kernel built from scratch (for AArch64)</p>

<img src="/Documentation/cover.png">
</div>

## Kernel Features
* Capable of running on a real Raspberry Pi 3B+
* AArch64 kernel with (user & kernel) preemptive multi-threading
* Copy-on-write `fork()`
* Virtual memory
* Virtual filesystem (VFS)
* FAT32 filesystem (supports long filenames)
* /dev, /proc, /tmp filesystem
* [Self-made C++ standard library](https://github.com/aesophor/valkyrie/tree/master/include/lib)

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
int sys_munmap(void __user *addr, size_t len);  // unfinished
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

## Build valkyrie
### Download `sd.img` from [here](https://drive.google.com/file/d/1oF4iG1EFJrHJnOFz9PepB5tiL2tyRfOY/view?usp=share_link).
The `sd.img` file contains:
* The boot partition (in which `kernel8.img` resides).
* The root partition (consists of `/bin`, `/usr`, etc).
* If you wish to run valkyrie in QEMU, then place `sd.img` under the project's root dir.
* If you wish to run valkyrie on a real rpi3b+, then flash it to your SD card with `dd`.

### Build requirements
* GNU make
* aarch64 (cross) compiler toolchain
* qemu-system-aarch64

### Installing ARMv8 (cross) compiler toolchain and QEMU
```sh
# Arch Linux (x86_64)
sudo pacman -S aarch64-linux-gnu-gcc aarch64-linux-gnu-gdb qemu-arch-extra

# macOS (x86_64)
brew tap messense/macos-cross-toolchains
brew install aarch64-unknown-linux-gnu qemu

# macOS (Apple Silicon)
brew install aarch64-elf-gcc aarch64-elf-binutils qemu
```

### Building valkyrie
```
git clone https://github.com/aesophor/valkyrie
cd valkyrie
make
```

## Run valkyrie
### Running valkyrie in QEMU
```
make run
```

### Running valkyrie on a real rpi3b+
Set up the USB-TO-TTL (USB-TO-SERIAL) Converter
| RPI3 Pin | USB-TO-TTL Pin |
| --- | --- |
| GND | GND |
| UART0 TX | RXD |
| UART0 RX | TXD |

![](https://docs.microsoft.com/en-us/windows/iot-core/media/pinmappingsrpi/rp2_pinout.png)

1. Flash `sd.img` to the SD card ([download](https://github.com/aesophor/valkyrie/#download-sdimg-from-here))
2. Mount the SD card and replace the `kernel8.img` on the SD card with the latest build.
3. Eject the SD card and plug it into RPI3.
4. Plug in the USB-TO-TTL converter to your computer
   - for macOS, run `screen /dev/tty.usbserial-0001 115200`
   - for linux, run `screen /dev/ttyUSB0 115200`

## References
* [國立陽明交通大學 資訊科學與工程研究所, Operating System Capstone, Spring 2021](https://grasslab.github.io/NYCU_Operating_System_Capstone/)
* [Linux](https://github.com/torvalds/linux)
* [SerenityOS](https://github.com/SerenityOS/serenity)
* [OS67](https://github.com/SilverRainZ/OS67)

## License
[GNU General Public License v3](https://github.com/aesophor/valkyrie/blob/309551004/LICENSE)
