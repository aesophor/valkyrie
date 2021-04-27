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
    : _runqueue(),
      _zombies() {}


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
  }

  /*
  auto& next = _runqueue.front();

  printf(">>>> context switch: %d -> %d\n", Task::get_current().get_pid(),
                                            next->get_pid());

  size_t sp;
  size_t lr;

  asm volatile("mov %0, sp" : "=r" (sp));
  asm volatile("mov %0, lr" : "=r" (lr));

  printf("current task: SP = 0x%x, LR = 0x%x\n", sp,
                                                 lr);

  printf("next task: SP = 0x%x, LR = 0x%x\n", next->_context.sp,
                                              next->_context.lr);
  */

  switch_to(&Task::get_current(), _runqueue.front().get());

  /*
  asm volatile("mov %0, sp" : "=r"(sp));
  asm volatile("mov %0, lr" : "=r" (lr));

  printf("<<<< return from pid: %d [%s], SP: 0x%x LR: 0x%x\n", Task::get_current().get_pid(),
                                            Task::get_current().get_name(),
                                            sp,
                                            lr);
  */
}

void TaskScheduler::tick() {
  Task::get_current().reduce_time_slice();
}

void TaskScheduler::terminate(Task& task) {
  task.set_state(Task::State::TERMINATED);
  _zombies.push_back(remove_task(task));
}

void TaskScheduler::reap_zombies() {
  if (_zombies.empty()) {
    return;
  }

  printk("sched: reaping %d zombie(s)\n", _zombies.size());
  _zombies.clear();
}

}  // namespace valkyrie::kernel
