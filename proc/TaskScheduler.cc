// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <dev/IO.h>
#include <dev/Console.h>
#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <usr/Shell.h>

extern "C" void switch_to(valkyrie::kernel::Task* prev,
                          valkyrie::kernel::Task* next);

namespace valkyrie::kernel {


void func() {
  for (int i = 0; i < 10; ++i) {
    printf("pid: %d %d\n", sys_getpid(), i);
    io::delay(1000000);
    TaskScheduler::get_instance().schedule();
  }
  printf("thread 0x%x is terminating...\n", &Task::get_current());
  sys_exit();
  Kernel::panic("NOOO\n");
}

int main(int argc, char** argv) {
  printf("Argv Test (pid %d), argc = %d, argv = 0x%x\n", sys_getpid(), argc, argv);
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  sys_exit();
  Kernel::panic("sys_exit failed\n");
  return 0;
}

void exec_test() {
  const char *argv[] = {"omg", "-o", "arg2", "holy_shit", nullptr};
  sys_exec(reinterpret_cast<void (*)()>(main), argv);
  Kernel::panic("sys_exec faild\n");
}

void fork_test() {
  printf("Fork Test, pid %d\n", sys_getpid());
  int cnt = 1;
  int ret = 0;
  if ((ret = sys_fork()) == 0) { // child
    printf("pid: %d, cnt: %d, ptr: 0x%x\n", sys_getpid(), cnt, &cnt);
    ++cnt;
    sys_fork();
    while (cnt < 5) {
      printf("pid: %d, cnt: %d, ptr: 0x%x\n", sys_getpid(), cnt, &cnt);
      io::delay(1000000);
      ++cnt;
    }
    printf("child terminating...\n");
  } else {
    printf("parent here, pid %d, child %d\n", sys_getpid(), ret);
    printf("parent terminating...\n");
  }

  sys_exit();
  Kernel::panic("?__?\n");
}

int argv_test(int argc, char** argv) {
  printf("Argv Test (pid %d), argc = %d, argv = 0x%x\n", sys_getpid(), argc, argv);
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  const char *fork_argv[] = {"fork_test", 0};
  sys_exec(fork_test, fork_argv);
  Kernel::panic("sys_exec failed\n");
  return 0;
}

void argv_test_driver() {
  const char *fork_argv[] = {"argv_test", 0};
  sys_exec(reinterpret_cast<void(*)()>(argv_test), fork_argv);
}

void idle() {
  while (true) {
    TaskScheduler::get_instance().reap_zombies();
    TaskScheduler::get_instance().schedule();
  }
}

TaskScheduler& TaskScheduler::get_instance() {
  static TaskScheduler instance;
  return instance;
}

TaskScheduler::TaskScheduler()
    : _run_queue(),
      _zombies() {}


void TaskScheduler::run() {
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(idle), "idle"));
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(argv_test_driver), "argv_test_driver"));

  if (_run_queue.empty()) {
    Kernel::panic("No working init found.\n");
  }
  switch_to(nullptr, _run_queue.front().get());
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  _run_queue.push_back(move(task));
  printk("scheduler: added a thread 0x%x [%s] (pid = %d)\n",
      _run_queue.back().get(),
      _run_queue.back()->get_name(),
      _run_queue.back()->get_pid());
}

UniquePtr<Task> TaskScheduler::remove_task(const Task& task) {
  printk("scheduler: removing thread 0x%x [%s] (pid = %d)\n",
      &task,
      task.get_name(),
      task.get_pid());

  UniquePtr<Task> removed_task;

  _run_queue.remove_if([&removed_task, &task](auto& t) {
    return t.get() == &task &&
           (removed_task = move(t), true);
  });

  return removed_task;
}


void TaskScheduler::schedule() {
  if (unlikely(_run_queue.empty())) {
    Kernel::panic("_run_queue is empty.\n");
  }

  if (likely(_run_queue.size() > 1)) {
    // Move the first task in runqueue to the end.
    auto task = move(_run_queue.front());
    _run_queue.pop_front();
    _run_queue.push_back(move(task));
  }

  /*
  printf("ctx switch: 0x%x (%s) -> 0x%x (%s): 0x%x\n",
      _run_queue.back().get(),
      _run_queue.back()->get_name(),
      _run_queue.front().get(),
      _run_queue.front()->get_name(),
      _run_queue.front()->_context.lr);
  */

  // Run the next task.
  switch_to(&Task::get_current(), _run_queue.front().get());
}

void TaskScheduler::mark_terminated(Task& task) {
  task.set_state(Task::State::TERMINATED);
  _zombies.push_back(remove_task(task));
}

void TaskScheduler::reap_zombies() {
  if (!_zombies.empty()) {
    printk("reaping %d zombies\n", _zombies.size());
    _zombies.clear();
  }
}

}  // namespace valkyrie::kernel
