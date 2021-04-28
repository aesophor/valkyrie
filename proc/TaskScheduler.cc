// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <proc/idle.h>
#include <proc/start_init.h>

namespace valkyrie::kernel {

TaskScheduler& TaskScheduler::get_instance() {
  static TaskScheduler instance;
  return instance;
}

TaskScheduler::TaskScheduler()
    : _need_reschedule(),
      _runqueue() {}


void TaskScheduler::run() {
  enqueue_task(make_unique<Task>(nullptr, idle, "idle"));
  enqueue_task(make_unique<Task>(nullptr, start_init, "init"));

  switch_to(/*prev=*/nullptr, /*next=*/_runqueue.front().get());
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  if (unlikely(!task)) {
    Kernel::panic("sched: task is empty\n");
  }

  printk("sched: adding thread to the runqueue 0x%x [%s] (pid = %d)\n",
      task.get(),
      task->get_name(),
      task->get_pid());

  task->set_state(Task::State::RUNNABLE);
  _runqueue.push_back(move(task));
}

UniquePtr<Task> TaskScheduler::remove_task(const Task& task) {
  printk("sched: removing thread from the runqueue 0x%x [%s] (pid = %d)\n",
      &task,
      task.get_name(),
      task.get_pid());

  UniquePtr<Task> removed_task;

  _runqueue.remove_if([&removed_task, &task](auto& t) {
    return t.get() == &task &&
           (removed_task = move(t), true);
  });

  if (unlikely(!removed_task)) {
    Kernel::panic("sched: removed_task is empty\n");
  }

  return removed_task;
}


void TaskScheduler::schedule() {
  // Maybe move the first task in the runqueue to the end.
  if (likely(_runqueue.size() > 1)) {
    auto task = move(_runqueue.front());
    _runqueue.pop_front();
    _runqueue.push_back(move(task));

    // FIXME: for debugging purpose only. Remove this block later.
    auto& next_task = _runqueue.front();
    printk(">>>> context switch: next: pid = %d [%s]\n", next_task->get_pid(),
                                                         next_task->get_name());
  }

  switch_to(&Task::get_current(), _runqueue.front().get());
}

void TaskScheduler::maybe_reschedule() {
  if (!_need_reschedule) {
    return;
  }

  _need_reschedule = false;
  Task::get_current().set_time_slice(TASK_TIME_SLICE);

  schedule();
}

void TaskScheduler::tick() {
  auto& current = Task::get_current();
  current.tick();

  if (current.get_time_slice() <= 0) {
    _need_reschedule = true;
  }
}

}  // namespace valkyrie::kernel
