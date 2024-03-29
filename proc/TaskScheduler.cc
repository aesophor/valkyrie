// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/TaskScheduler.h>

#include <Mutex.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

TaskScheduler::TaskScheduler() : _need_reschedule(), _runqueue() {}

void TaskScheduler::run() {
  if (_runqueue.empty()) [[unlikely]] {
    Kernel::panic("No tasks in runqueue!\n");
  }

  // Switch to the first task.
  switch_to(/*prev=*/nullptr, /*next=*/_runqueue.front().get());
}

void TaskScheduler::enqueue_task(UniquePtr<Task> task) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  if (!task) [[unlikely]] {
    Kernel::panic("sched: task is empty\n");
  }

#ifdef DEBUG
  printk("sched: adding thread to the runqueue 0x%x [%s] (pid = %d)\n", task.get(),
         task->get_name(), task->get_pid());
#endif

  task->set_state(Task::State::RUNNING);
  _runqueue.push_back(move(task));
}

UniquePtr<Task> TaskScheduler::remove_task(const Task &task) {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  UniquePtr<Task> removed_task;

#ifdef DEBUG
  printk("sched: removing thread from the runqueue 0x%x [%s] (pid = %d)\n", &task,
         task.get_name(), task.get_pid());
#endif

  _runqueue.remove_if([&removed_task, &task](auto &t) {
    return t.get() == &task && (removed_task = move(t), true);
  });

  if (!removed_task) [[unlikely]] {
    Kernel::panic("sched: removed_task is empty\n");
  }

  return removed_task;
}

void TaskScheduler::schedule() {
  const LockGuard<RecursiveMutex> lock(Kernel::mutex);

  // Maybe move the first task in the runqueue to the end.
  if (_runqueue.size() > 1) [[likely]] {
    auto task = move(_runqueue.front());
    _runqueue.pop_front();
    _runqueue.push_back(move(task));

#ifdef DEBUG
    auto &next_task = _runqueue.front();
    printf(">>>> context switch: next: pid = %d [%s], SP = 0x%p\n", next_task->get_pid(),
           next_task->get_name(), next_task->_context.sp);
#endif
  }

  // Update task states.
  Task::current()->set_state(Task::State::SLEEPING);
  _runqueue.front()->set_state(Task::State::RUNNING);

  switch_to(Task::current(), _runqueue.front().get());
}

void TaskScheduler::maybe_schedule() {
  if (!_need_reschedule) {
    return;
  }

  _need_reschedule = false;
  Task::current()->set_time_slice(TASK_TIME_SLICE);

#ifdef DEBUG
  printk("Preempting current task @0x%p... (pid = %d)\n", Task::current(),
         Task::current()->get_pid());
#endif

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
