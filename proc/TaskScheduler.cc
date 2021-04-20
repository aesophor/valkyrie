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
  sys_exit();
}

void omg() {
  printf("omg\n");
  sys_exit();
}

void exec_test() {
  const char *argv[] = {"omg", "-o", "arg2", nullptr};
  sys_exec(omg, argv);
  sys_exit();
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
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(func), "func"));

  if (_run_queue.empty()) {
    Kernel::panic("No working init found.\n");
  }
  switch_to(nullptr, _run_queue.front().get());
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  _run_queue.push_back(move(task));
  printk("scheduler: added a thread 0x%x\n", _run_queue.back().get());
}

UniquePtr<Task> TaskScheduler::remove_task(const Task& task) {
  printk("scheduler: removing thread 0x%x (pid = %d)\n", &task, task.get_pid());

  UniquePtr<Task> removed_task;

  _run_queue.remove_if([&removed_task, &task](auto& t) {
    return t.get() == &task &&
           (removed_task = move(t), true);
  });
  return removed_task;
}

void TaskScheduler::mark_as_zombie(Task& task) {
  printk("marking 0x%x as zombie\n", &task);
  task.set_state(Task::State::ZOMBIE);
  _zombies.push_back(remove_task(task));
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

  // Run the next task.
  switch_to(_run_queue.back().get(), _run_queue.front().get());
}

void TaskScheduler::reap_zombies() {
  if (!_zombies.empty()) {
    printk("reaping %d zombies\n", _zombies.size());
    _zombies.clear();
  }
}

}  // namespace valkyrie::kernel
