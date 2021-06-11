<div align="center">

<h3>VALKYRIE</h3>
<p>64-bit unix-like kernel for Raspberry Pi 3B+ (CPU: Arm Cortex A53)</p>

<img src="/Documentation/cover-vm.jpeg">
</div>

<br>

## [NYCU, Operating System Capstone, Spring 2021](https://grasslab.github.io/NYCU_Operating_System_Capstone/index.html)

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
| `309551004` | `aesophor` | `王冠中` | aesophor.cs09g [at] nctu.edu.tw |

## Progress Overview

- [x] Lab1: Hello World
- [x] Lab2: Booting ([bootloader](https://github.com/aesophor/valkyrie/tree/lab2-bootloader))
- [x] Lab3: Allocator (pmm)
- [x] Lab4: Exception and Interrupt Handling
- [x] Lab5: Multitasking
- [x] Lab6: Virtual Filesystem
- [x] Lab7: Filesystem Meets Hardware
- [x] Lab8: Virtual Memory (vmm)

## Kernel Features
- [x] I/O: MiniUART - supports sync/async I/O
- [x] ARM Mailbox API
- [x] Exception & interrupt handling - top/bottom halves, tasklets
- [x] Buddy allocator
- [x] Dynamic allocator - my own optimized SLOB allocator (a simplified `ptmalloc`)
- [ ] Boot memory allocator
- [x] User / Kernel threads
- [x] Multitasking - sys_fork(), sys_exec(), sys_wait(), sys_exit()
- [x] User tasks preemption
- [ ] Kernel preemption - protect critical sections
- [x] POSIX signals - sys_kill(), sys_signal()
- [ ] POSIX custom signal handlers - sys_rt_sigreturn()
- [ ] Wait Queues
- [x] tmpfs
- [x] Virtual filesystem (VFS)
- [x] System-wide opened file table, Per-process file descriptor tables
- [x] POSIX file I/O - sys_read(), sys_write(), sys_open(), sys_close()
- [x] Multi-level VFS - sys_chdir(), sys_mkdir(), sys_mount(), sys_umount()
- [ ] Procfs
- [x] Parse MBR (Master Boot Record)
- [x] FAT32 with LFN (Long File Name) support - open, read, write, close
- [x] Character device, Block device
- [x] Device file, mknod, device files
- [ ] Component name cache mechanism for faster pathname lookup
- [ ] Page cache mechanism for faster file r/w
- [ ] sync() - write back cache data
- [x] kernel virtual address space
- [x] user virtual address space
- [ ] mmap
- [x] ELF parser and loader (incomplete)
- [ ] Page fault handler, Demand paging
- [ ] Copy on Write
- [ ] ...

## Kernel C++20 STL Progress

- [ ] Algorithm
- [ ] Concepts
- [x] Functional
- [x] Iterator
- [x] UniquePtr + make_unique<>()
- [x] SharedPtr + make_shared<>() + \*_pointer_cast<>()
- [ ] WeakPtr
- [x] List
- [x] String
- [ ] Vector (?)
- [x] Deque (the performance is shit ...)
- [x] Utility (move and forward)
- [ ] ...

## Build and Deploy

* [BUILD.md](https://github.com/aesophor/valkyrie/blob/309551004/Documentation/BUILD.md)
* [DEPLOY.md](https://github.com/aesophor/valkyrie/blob/309551004/Documentation/DEPLOY.md)

## Thanks

* [Linux](https://github.com/torvalds/linux)
* [SerenityOS](https://github.com/SerenityOS/serenity)
* [OS67](https://github.com/SilverRainZ/OS67)

## License
Available under the [MIT License](https://github.com/aesophor/valkyrie/blob/309551004/LICENSE)
