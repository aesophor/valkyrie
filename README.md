## [NYCU, Operating System Capstone, Spring 2021](https://grasslab.github.io/NYCU_Operating_System_Capstone/index.html)

A 64-bit Unix-like operating system which runs on Raspberry Pi 3b+ (CPU: Arm Cortex A53, ARMv8)

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
| `309551004` | `aesophor` | `王冠中` | aesophor.cs09g [at] nctu.edu.tw |

<div align="center">
<img src="/Documentation/cover.png">
</div>

<br>

## Progress Overview

- [x] Lab1: Hello World
- [x] Lab2: Booting ([bootloader](https://github.com/aesophor/valkyrie/tree/lab2-bootloader))
- [x] Lab3: Allocator (pmm)
- [x] Lab4: Exception and Interrupt Handling
- [x] Lab5: Multitasking
- [x] Lab6: Virtual Filesystem
- [x] Lab7: Filesystem Meets Hardware
- [ ] Lab8: Virtual Memory (vmm)

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
- [ ] POSIX signals and custom signal handlers - sys_kill(), sys_signal(), sys_rt_sigreturn()
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
- [ ] user virtual address space
- [ ] mmap
- [ ] ELF parser and loader
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

## License
Available under the [MIT License](https://github.com/aesophor/valkyrie/blob/309551004/LICENSE)
