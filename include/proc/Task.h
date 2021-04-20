// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TASK_H_
#define VALKYRIE_TASK_H_

#include <Types.h>
#include <mm/PageFrameAllocator.h>

#define TASK_0_STACK_TOP HEAP_END
#define TASK_STACK_SIZE  8192

namespace valkyrie::kernel {

class Task {
 public:
  enum class State {
    CREATED,
    RUNNABLE,
    WAITING,
    ZOMBIE,
    SIZE
  };

  // Constructor
  Task(void *entry_point, const char* name);

  // Destructor
  ~Task();


  int exec(void (*func)(), const char* const argv[]);

  static Task& get_current();
  static void  set_current(const Task* t);

  void set_state(Task::State state) {
    _state = state;
  }

  uint32_t get_pid() const;

  int get_time_slice() const {
    return _time_slice;
  }

  void reduce_time_slice() {
    _time_slice -= 1;
  }

  const char* get_name() const {
    return _name;
  }


 private:
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
    uint64_t pc;
  } _context;

  Task::State _state;
  uint32_t _pid;
  int _time_slice;
  void* _entry_point;
  void* _stack_page;
  char _name[16];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TASK_H_
