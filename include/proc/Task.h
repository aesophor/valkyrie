// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <List.h>
#include <Memory.h>
#include <Types.h>
#include <dev/Console.h>
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
 public:
  friend class TaskScheduler;

  enum class State {
    CREATED,
    RUNNABLE,
    WAITING,
    TERMINATED,
    SIZE
  };

  // Constructor
  template <typename T>
  Task(Task* parent, T entry_point, const char* name)
      : _context(),
        _parent(parent),
        _active_children(),
        _terminated_children(),
        _state(Task::State::CREATED),
        _error_code(),
        _pid(Task::_next_pid++),
        _time_slice(TASK_TIME_SLICE),
        _entry_point(reinterpret_cast<void*>(entry_point)),
        _elf_dest(),
        _kstack_page(get_free_page()),
        _ustack_page(get_free_page()),
        _name(),
        _pending_signals(),
        _custom_signal_handlers() {
    if (unlikely(_pid == 1)) {
      Task::_init = this;
    } else if (unlikely(_pid == 2)) {
      Task::_kthreadd = this;
    }

    if (parent) {
      parent->_active_children.push_back(this);
    }

    _context.lr = reinterpret_cast<uint64_t>(entry_point);
    _context.sp = _kstack_page.end();
    strcpy(_name, name);

    printk("constructed thread 0x%x [%s] (pid = %d): entry: 0x%x\n",
        this,
        _name,
        _pid,
        _entry_point);
  }


  // Destructor
  ~Task() {
    printk("destructing thread 0x%x [%s] (pid = %d)\n",
        this,
        _name,
        _pid);

    // If the current task still has running children,
    // make the init task adopt these orphans.
    // Note: terminated children will be automatically released.
    while (!_active_children.empty()) {
      auto child = _active_children.front();
      get_init()._active_children.push_back(child);
      _active_children.pop_front();
    }

    kfree(_kstack_page.get());
    kfree(_ustack_page.get());
  }


  // Copy constructor
  Task(const Task& r) = delete;

  // Copy assignment operator
  Task& operator= (const Task& r) = delete;



  static Task& get_current() {
    Task* current;
    asm volatile("mrs %0, TPIDR_EL1" : "=r" (current));
    return *current;
  }

  static void set_current(const Task* t) {
    asm volatile("msr TPIDR_EL1, %0" :: "r" (t));
  }

  static Task& get_init() {
    return *Task::_init;
  }

  static Task& get_kthreadd() {
    return *Task::_kthreadd;
  }

  static Task* get_by_pid(const pid_t pid);

  [[gnu::always_inline]] void save_context() {
    switch_to(this, nullptr);
  }


  int do_fork();
  int do_exec(const char* name, const char* const _argv[]);
  int do_wait(int* wstatus);
  [[noreturn]] void do_exit(int error_code);
  long do_kill(pid_t pid, Signal signal);
  int do_signal(int signal, void (*handler)());

  void handle_pending_signals();


  Task::State get_state() const { return _state; }
  pid_t get_pid() const { return _pid; }
  TrapFrame* get_trap_frame() const { return _trap_frame; }
  const char* get_name() const { return _name; }

  void set_state(Task::State state) { _state = state; }
  void set_trap_frame(TrapFrame* trap_frame) { _trap_frame = trap_frame; }

  int get_time_slice() const { return _time_slice; }
  void set_time_slice(int time_slice) { _time_slice = time_slice; }

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
  

 private:
  size_t construct_argv_chain(const char* const _argv[]);
  int probe_for_argc(const char* const argv[]) const;

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
  void* _entry_point;
  void* _elf_dest;
  Page _kstack_page;
  Page _ustack_page;
  TrapFrame* _trap_frame;
  char _name[TASK_NAME_MAX_LEN];

  // POSIX signals
  List<Signal> _pending_signals;
  void (*_custom_signal_handlers[Signal::__NR_signals])();
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_
