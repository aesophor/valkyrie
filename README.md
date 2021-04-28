## NYCU, Operating System Capstone, Spring 2021

Lab Specification: https://grasslab.github.io/NYCU_Operating_System_Capstone/index.html

| 學號 | GitHub 帳號 | 姓名 | Email |
| --- | ----------- | --- | --- |
| `309551004` | `aesophor` | `王冠中` | aesophor.cs09g@nctu.edu.tw |

## Kernel Progress

- [x] Bootloader [see tag: lab2-bootloader](https://github.com/aesophor/valkyrie/tree/lab2-bootloader)
- [x] Exception/Interrupt handling
- [x] System calls
- [x] Buddy allocator
- [x] Dynamic allocator: my own optimized SLOB allocator (a simplified `ptmalloc`)
- [ ] Boot memory allocator
- [x] Preemptive multitasking (supports user/kernel threads)
- [x] fork(), exec(), wait(), exit()
- [ ] POSIX signals
- [ ] Virtual filesystem
- [ ] Virtual memory

## Kernel STL Progress

- [ ] Algorithms
- [ ] Concepts
- [x] Functional
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
