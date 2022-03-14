// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <Concepts.h>
#include <List.h>
#include <Memory.h>
#include <Mutex.h>
#include <TypeTraits.h>
#include <Types.h>
#include <Utility.h>

#include <fs/ELF.h>
#include <fs/File.h>
#include <fs/Vnode.h>
#include <mm/Page.h>
#include <mm/UserspaceAccess.h>
#include <mm/VirtualMemoryMap.h>
#include <proc/Signal.h>

#define TASK_TIME_SLICE 64
#define TASK_NAME_MAX_LEN 16
#define NR_TASK_FD_LIMITS 16

// mmap() prots
#define PROT_NONE 0x0  /* Page can not be accessed. */
#define PROT_READ 0x1  /* Page can be read. */
#define PROT_WRITE 0x2 /* Page can be written. */
#define PROT_EXEC 0x4  /* Page can be executed. */

// mmap() sharing types (must choose one and only one of these).
#define MAP_SHARED 0x01  /* Share changes. */
#define MAP_PRIVATE 0x02 /* Changes are private. */
#define MAP_TYPE 0x0f    /* Mask for type of mapping. */

// mmap() flags
#define MAP_FIXED 0x10       /* Interpret addr exactly. */
#define MAP_ANONYMOUS 0x20   /* Don't use a file. */
#define MAP_POPULATE 0x08000 /* Populate (prefault) pagetables. */

namespace valkyrie::kernel {

// Forward declaration.
class Task;
class TrapFrame;

extern "C" void switch_to(Task *prev, Task *next);
extern "C" void switch_to_user_mode(void *entry_point, size_t user_sp, size_t kernel_sp,
                                    void *page_table);

class Task {
  // To copy (duplicate) a task, use Task::do_fork().
  MAKE_NONCOPYABLE(Task);
  MAKE_NONMOVABLE(Task);

  // Friend declaration
  friend class TaskScheduler;

 public:
  enum class State { CREATED, RUNNING, SLEEPING, TERMINATED, SIZE };

  // Constructor
  Task(bool is_user_task, Task *parent, void (*entry_point)(), const char *name);

  // Destructor
  ~Task();

  [[gnu::always_inline]] inline static Task *current() {
    Task *ret;
    asm volatile("mrs %0, TPIDR_EL1" : "=r"(ret));
    return ret;
  }

  [[gnu::always_inline]] inline void save_context() {
    switch_to(this, nullptr);
  }

  static List<Task *> get_active_tasks();
  static Task *get_by_pid(const pid_t pid);

  int fork();
  int exec(const char *name, const char *const _argv[]);
  int wait(int *wstatus);
  [[noreturn]] void exit(int error_code);
  long kill(pid_t pid, Signal signal);
  int signal(int signal, void (*handler)());
  void __user *mmap(void __user *addr, size_t len, int prot, int flags, int fd,
                    int file_offset);
  void __user *do_mmap(void __user *addr, size_t len, int prot, int flags,
                       SharedPtr<File> file, int file_offset,
                       size_t zero_page_file_offset = -1);
  int munmap(void *addr, size_t len);

  void handle_pending_signals();

  int allocate_fd_for_file(SharedPtr<File> file);
  SharedPtr<File> release_fd_and_get_file(const int fd);
  SharedPtr<File> get_file_by_fd(const int fd) const;
  bool is_fd_valid(const int fd) const;

  bool is_user_task() const {
    return _is_user_task;
  }

  Task::State get_state() const {
    return _state;
  }

  pid_t get_pid() const {
    return _pid;
  }

  TrapFrame *get_trap_frame() const {
    return _trap_frame;
  }

  const char *get_name() const {
    return _name;
  }

  void set_state(Task::State state) {
    _state = state;
  }

  void set_trap_frame(TrapFrame *trap_frame) {
    _trap_frame = trap_frame;
  }

  int get_time_slice() const {
    return _time_slice;
  }

  void set_time_slice(int time_slice) {
    _time_slice = time_slice;
  }

  void tick() {
    if (_time_slice > 0) {
      _time_slice--;
    }
  }

  size_t get_children_count() const {
    return _active_children.size() + _terminated_children.size();
  }

  size_t get_active_children_count() const {
    return _active_children.size();
  }

  size_t get_terminated_children_count() const {
    return _terminated_children.size();
  }

  SharedPtr<Vnode> get_cwd_vnode() const {
    return _cwd_vnode;
  }

  void set_cwd_vnode(SharedPtr<Vnode> vnode) {
    _cwd_vnode = move(vnode);
  }

  const VMMap &get_vmmap() const {
    return _vmmap;
  }

  size_t *get_ttbr0_el1() const {
    return _vmmap.get_pgd();
  }

  // Converts the virtual address to physical address by looking up the page table.
  template <Pointer T>
  T v2p(T v_addr) {
    auto addr = reinterpret_cast<size_t>(v_addr);
    return reinterpret_cast<T>(_vmmap.get_physical_address(addr));
  }

 private:
  // Loads the specified ELF file into the virtual address space of this task.
  // XXX: We should take `elf` by const reference...
  bool load_elf_binary(SharedPtr<File> file, ELF &elf);
  void map_elf_segment(SharedPtr<File> file, const ELF &elf, const ELF::Segment &segment);

  // Copies the arguments into the bottom of the user stack of this task.
  size_t copy_arguments_to_user_stack(const char *const _argv[]);

  static Task *_init;
  static Task *_kthreadd;
  static pid_t _next_pid;

  // For now we keep this as the first member of Task, so that
  // proc/ctx_switch.S can access process context directly.
  struct Context {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;
    uint64_t lr;
    uint64_t sp;
  } _context;

  bool _is_user_task;
  Task *_parent;
  List<Task *> _active_children;
  List<UniquePtr<Task>> _terminated_children;
  Task::State _state;
  int _error_code;
  pid_t _pid;
  int _time_slice;
  VMMap _vmmap;
  void (*_entry_point)();
  Page _kstack_page;
  Page _ustack_page;
  TrapFrame *_trap_frame;
  char _name[TASK_NAME_MAX_LEN];

  // POSIX signals
  List<Signal> _pending_signals;
  void (*_custom_signal_handlers[Signal::__NR_signals])();

  // Per-process file descriptors
  SharedPtr<File> _fd_table[NR_TASK_FD_LIMITS];

  // Current working directory
  SharedPtr<Vnode> _cwd_vnode;
};


template <typename... Args>
static UniquePtr<Task> make_kernel_task(Args &&...args) {
  return make_unique<Task>(false, forward<Args>(args)...);
}

template <typename... Args>
static UniquePtr<Task> make_user_task(Args &&...args) {
  return make_unique<Task>(true, forward<Args>(args)...);
}

// Built-in tasks entry points.
[[noreturn]] void idle();
[[noreturn]] void start_init();
[[noreturn]] void start_kthreadd();

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_
