// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <dev/IO.h>
#include <dev/Console.h>
#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <proc/start_init.h>

extern "C" void switch_to(valkyrie::kernel::Task* prev,
                          valkyrie::kernel::Task* next);

namespace valkyrie::kernel {

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
  enqueue_task(make_unique<Task>(reinterpret_cast<void*>(start_init), "init"));

  switch_to(nullptr, _run_queue.front().get());
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  if (unlikely(!task)) {
    Kernel::panic("sched: task is empty\n");
  }

  printk("sched: adding thread to the runqueue 0x%x [%s] (pid = %d)\n",
      task->get_name(),
      task->get_pid());

  task->set_state(Task::State::RUNNABLE);
  _run_queue.push_back(move(task));
}

UniquePtr<Task> TaskScheduler::remove_task(const Task& task) {
  printk("sched: removing thread from the runqueue 0x%x [%s] (pid = %d)\n",
      &task,
      task.get_name(),
      task.get_pid());

  UniquePtr<Task> removed_task;

  _run_queue.remove_if([&removed_task, &task](auto& t) {
    return t.get() == &task &&
           (removed_task = move(t), true);
  });

  if (unlikely(!removed_task)) {
    Kernel::panic("sched: removed_task is empty\n");
  }

  return removed_task;
}


void TaskScheduler::schedule() {
  if (unlikely(_run_queue.empty())) {
    Kernel::panic("sched: runqueue is empty.\n");
  }

  // Maybe move the first task in the runqueue to the end.
  if (likely(_run_queue.size() > 1)) {
    auto task = move(_run_queue.front());
    _run_queue.pop_front();
    _run_queue.push_back(move(task));
  }

  switch_to(&Task::get_current(), _run_queue.front().get());
}

void TaskScheduler::mark_terminated(Task& task) {
  task.set_state(Task::State::TERMINATED);
  _zombies.push_back(remove_task(task));
}

void TaskScheduler::reap_zombies() {
  if (!_zombies.empty()) {
    printk("sched: reaping %d zombies\n", _zombies.size());
    _zombies.clear();
  }
}

}  // namespace valkyrie::kernel
