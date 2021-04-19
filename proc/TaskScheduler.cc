// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <dev/IO.h>
#include <dev/Console.h>
#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <usr/Shell.h>

extern "C" void switch_to(valkyrie::kernel::Task* prev,
                          valkyrie::kernel::Task* next);

namespace valkyrie::kernel {

void exit() {
  // Mark current thread as zombie process,
  // remove it from the _run_queue, and
  // add it to _zombie_list.
  TaskScheduler::get_instance().mark_as_zombie(Task::get_current());
}

void func() {
  for (int i = 0; i < 10; ++i) {
    printf("pid: %d %d\n", Task::get_current().get_pid(), i);
    io::delay(1000000);
    TaskScheduler::get_instance().schedule();
  }
  exit();
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
  //printk("scheduler: added shell task\n");
  //enqueue_task(make_unique<Task>(reinterpret_cast<void*>(run_shell)));
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(idle)));
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(func)));
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(func)));

  if (_run_queue.empty()) {
    Kernel::panic("No working init found\n");
  }

  switch_to(nullptr, _run_queue.front().get());
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  printk("scheduler: added a thread\n");
  _run_queue.push_back(move(task));
}

void TaskScheduler::remove_task(Task* task) {
  printk("scheduler: removing thread (pid = %d)\n", task->get_pid());
  _run_queue.remove_if([task](const auto& t) {
    return t.get() == task;
  });
}

void TaskScheduler::mark_as_zombie(Task& task) {
  task.set_state(Task::State::ZOMBIE);
  _zombies.push_back(&task);
}

void TaskScheduler::schedule() {
  // Move the first task in runqueue to the end.
  auto task = move(_run_queue.front());
  _run_queue.pop_front();
  _run_queue.push_back(move(task));

  // Run the next task.
  switch_to(_run_queue.back().get(), _run_queue.front().get());
}

void TaskScheduler::reap_zombies() {
  while (!_zombies.empty()) {
    Task* task = _zombies.front();
    remove_task(task);
    _zombies.pop_front();
  }
}

}  // namespace valkyrie::kernel
