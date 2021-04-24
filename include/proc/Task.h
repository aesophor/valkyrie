// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <Types.h>
#include <mm/PageFrameAllocator.h>
#include <proc/TrapFrame.h>

#define TASK_0_STACK_TOP HEAP_END
#define TASK_STACK_SIZE  8192

namespace valkyrie::kernel {

class Task;
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
  Task(Task* parent, void* entry_point, const char* name);

  // Destructor
  ~Task();


  static Task& get_current();
  static void  set_current(const Task* t);

  [[gnu::always_inline]] void save_context() { switch_to(this, nullptr); }

  int fork();
  int exec(const char* name, const char* const _argv[]);
  void exit();

  Task::State get_state() const { return _state; }
  uint32_t get_pid() const { return _pid; }
  TrapFrame* get_trap_frame() const { return _trap_frame; }
  const char* get_name() const { return _name; }

  void set_state(Task::State state) { _state = state; }
  void set_trap_frame(TrapFrame* trap_frame) { _trap_frame = trap_frame; }

  int get_time_slice() const { return _time_slice; }
  void reduce_time_slice() { --_time_slice; }


 private:
  void construct_argv_chain(const char* const _argv[]);

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
  void* _kstack_page;
  void* _ustack_page;
  TrapFrame* _trap_frame;
  char _name[16];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_
