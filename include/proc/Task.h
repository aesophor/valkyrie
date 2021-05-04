// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <List.h>
#include <Memory.h>
#include <Types.h>
#include <dev/Console.h>
#include <fs/FileDescriptorTable.h>
#include <kernel/Compiler.h>
#include <libs/CString.h>
#include <mm/Page.h>
#include <mm/MemoryManager.h>
#include <proc/Signal.h>
#include <proc/TrapFrame.h>

#define TASK_TIME_SLICE    3
#define TASK_NAME_MAX_LEN 16

namespace valkyrie::kernel {

// Forward declaration.
class Task;
class TrapFrame;

extern "C" void switch_to(Task* prev, Task* next);

class Task {
  // Friend declaration
  friend class TaskScheduler;

 public:
  enum class State {
    CREATED,
    RUNNABLE,
    WAITING,
    TERMINATED,
    SIZE
  };

  // Constructor
  Task(Task* parent, void (*entry_point)(), const char* name);

  // Destructor
  ~Task();

  // Copy constructor
  Task(const Task& r) = delete;

  // Copy assignment operator
  Task& operator= (const Task& r) = delete;


  [[gnu::always_inline]] static Task& get_current() {
    Task* current;
    asm volatile("mrs %0, TPIDR_EL1" : "=r" (current));
    return *current;
  }

  [[gnu::always_inline]] static void set_current(const Task* t) {
    asm volatile("msr TPIDR_EL1, %0" :: "r" (t));
  }

  [[gnu::always_inline]] void save_context() {
    switch_to(this, nullptr);
  }

  static Task* get_by_pid(const pid_t pid);

  int do_fork();
  int do_exec(const char* name, const char* const _argv[]);
  int do_wait(int* wstatus);
  [[noreturn]] void do_exit(int error_code);
  long do_kill(pid_t pid, Signal signal);
  int do_signal(int signal, void (*handler)());

  void handle_pending_signals();

  int allocate_one_file_descriptor(SharedPtr<File> file);
  void deallocate_file_descriptor(const int fd);


  Task::State get_state() const { return _state; }
  pid_t get_pid() const { return _pid; }
  TrapFrame* get_trap_frame() const { return _trap_frame; }
  const char* get_name() const { return _name; }

  void set_state(Task::State state) { _state = state; }
  void set_trap_frame(TrapFrame* trap_frame) { _trap_frame = trap_frame; }

  int get_time_slice() const { return _time_slice; }
  void set_time_slice(int time_slice) { _time_slice = time_slice; }

  void tick() { if (_time_slice > 0) _time_slice--; }


  size_t get_children_count() const {
    return _active_children.size() + _terminated_children.size();
  }

  size_t get_active_children_count() const {
    return _active_children.size();
  }

  size_t get_terminated_children_count() const {
    return _terminated_children.size();
  }
  

 private:
  size_t copy_arguments_to_user_stack(const char* const _argv[]);

  static Task* _init;
  static Task* _kthreadd;
  static pid_t _next_pid;


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

  Task* _parent;
  List<Task*> _active_children;
  List<UniquePtr<Task>> _terminated_children;
  Task::State _state;
  int _error_code;
  pid_t _pid;
  int _time_slice;
  void (*_entry_point)();
  void* _elf_dest;
  Page _kstack_page;
  Page _ustack_page;
  TrapFrame* _trap_frame;
  char _name[TASK_NAME_MAX_LEN];

  // POSIX signals
  List<Signal> _pending_signals;
  void (*_custom_signal_handlers[Signal::__NR_signals])();

  // Per-process file descriptors
  FileDescriptorTable _fd_table;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_
