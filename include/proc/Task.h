// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <Types.h>
#include <dev/Console.h>
#include <libs/CString.h>
#include <mm/Page.h>
#include <mm/MemoryManager.h>
#include <proc/TrapFrame.h>

namespace valkyrie::kernel {

// Forward declaration.
class Task;
class TrapFrame;

extern "C" void switch_to(Task* prev, Task* next);

class Task {
 public:
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
        _state(Task::State::CREATED),
        _pid(Task::_next_pid++),
        _time_slice(3),
        _entry_point(reinterpret_cast<void*>(entry_point)),
        _elf_dest(),
        _kstack_page(get_free_page()),
        _ustack_page(get_free_page()),
        _name() {
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

    kfree(_kstack_page.get());
    kfree(_ustack_page.get());
  }


  static Task& get_current() {
    Task* current;
    asm volatile("mrs %0, TPIDR_EL1" : "=r" (current));
    return *current;
  }

  static void set_current(const Task* t) {
    asm volatile("msr TPIDR_EL1, %0" :: "r" (t));
  }

  [[gnu::always_inline]] void save_context() {
    switch_to(this, nullptr);
  }


  int fork();
  int exec(const char* name, const char* const _argv[]);
  [[noreturn]] void exit();


  Task::State get_state() const { return _state; }
  uint32_t get_pid() const { return _pid; }
  TrapFrame* get_trap_frame() const { return _trap_frame; }
  const char* get_name() const { return _name; }

  void set_state(Task::State state) { _state = state; }
  void set_trap_frame(TrapFrame* trap_frame) { _trap_frame = trap_frame; }

  int get_time_slice() const { return _time_slice; }
  void reduce_time_slice() { --_time_slice; }


 private:
  size_t construct_argv_chain(const char* const _argv[]);

  static uint32_t _next_pid;


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
  Task::State _state;
  uint32_t _pid;
  int _time_slice;
  void* _entry_point;
  void* _elf_dest;
  Page _kstack_page;
  Page _ustack_page;
  TrapFrame* _trap_frame;
  char _name[16];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_
