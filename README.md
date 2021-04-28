## NYCU, Operating System Capstone, Spring 2021

Lab Specification: https://grasslab.github.io/NYCU_Operating_System_Capstone/index.html

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
| `309551004` | `aesophor` | `王冠中` | aesophor.cs09g@nctu.edu.tw |

## Progress Overview

- [x] Lab1: Hello World
- [x] Lab2: Booting ([bootloader](https://github.com/aesophor/valkyrie/tree/lab2-bootloader))
- [x] Lab3: Allocator (pmm)
- [x] Lab4: Exception and Interrupt Handling
- [x] Lab5: Multitasking
- [ ] Lab6: Virtual Filesystem
- [ ] Lab7: Filesystem Meets Hardware
- [ ] Lab8: Virtual Memory (vmm)

## Kernel Features
- [x] I/O: MiniUART (with optional r/w buffers and interrupts)
- [x] ARM Mailbox
- [x] Exception & interrupt handling (top/bottom halves)
- [x] System calls
- [x] Buddy allocator
- [x] Dynamic allocator: my own optimized SLOB allocator (a simplified `ptmalloc`)
- [ ] Boot memory allocator
- [x] Preemptive multitasking (supports user/kernel threads)
- [x] fork(), exec(), wait(), exit()
- [ ] POSIX signals
- [ ] ...

## Kernel C++20 STL Progress

- [ ] Algorithms
- [ ] Concepts
- [x] Functional
- [x] Iterator (just a very basic one ...)
- [x] UniquePtr + make_unique<>()
- [x] SharedPtr + make_shared<>() + \*_pointer_cast<>()
- [ ] WeakPtr
- [x] DoublyLinkedList
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
