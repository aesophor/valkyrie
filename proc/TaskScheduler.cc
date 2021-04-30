// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <dev/Console.h>
#include <kernel/Compiler.h>
#include <kernel/ExceptionManager.h>
#include <kernel/Kernel.h>
#include <proc/idle.h>
#include <proc/start_init.h>
#include <proc/start_kthreadd.h>

namespace valkyrie::kernel {

TaskScheduler& TaskScheduler::get_instance() {
  static TaskScheduler instance;
  return instance;
}

TaskScheduler::TaskScheduler()
    : _need_reschedule(),
      _runqueue() {}


void TaskScheduler::run() {
  // Enqueue initial tasks.
  enqueue_task(make_unique<Task>(/*parent=*/nullptr, idle, "idle"));
  enqueue_task(make_unique<Task>(/*parent=*/nullptr, start_init, "init"));
  enqueue_task(make_unique<Task>(/*parent=*/nullptr, start_kthreadd, "kthreadd"));

  // Switch to the first task.
  switch_to(/*prev=*/nullptr, /*next=*/_runqueue.front().get());
}


void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  if (unlikely(!task)) {
    Kernel::panic("sched: task is empty\n");
  }

  /*
  printk("sched: adding thread to the runqueue 0x%x [%s] (pid = %d)\n",
      task.get(),
      task->get_name(),
      task->get_pid());
  */

  task->set_state(Task::State::RUNNABLE);
  _runqueue.push_back(move(task));
}

UniquePtr<Task> TaskScheduler::remove_task(const Task& task) {
  /*
  printk("sched: removing thread from the runqueue 0x%x [%s] (pid = %d)\n",
      &task,
      task.get_name(),
      task.get_pid());
  */

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

    /*
    auto& next_task = _runqueue.front();
    printf(">>>> context switch: next: pid = %d [%s], SP = 0x%x\n",
                                                         next_task->get_pid(),
                                                         next_task->get_name(),
                                                         next_task->_context.sp);
    */
  }

  switch_to(Task::current(), _runqueue.front().get());
}

void TaskScheduler::maybe_reschedule() {
  if (!_need_reschedule) {
    return;
  }

  _need_reschedule = false;
  Task::current()->set_time_slice(TASK_TIME_SLICE);

  schedule();
}

void TaskScheduler::tick() {
  auto current = Task::current();
  current->tick();

  if (current->get_time_slice() <= 0) {
    _need_reschedule = true;
  }
}

}  // namespace valkyrie::kernel
