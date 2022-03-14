// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Signal.h>

#include <proc/Task.h>

namespace valkyrie::kernel {

#define UNDEFINED sig_unsupported_handler

// clang-format off
void (*__default_signal_handler_table[Signal::__NR_signals])(int) = {
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    sigkill_default_handler,
    UNDEFINED,
    sigsegv_default_handler,
};
// clang-format on


void sigkill_default_handler(int signal) {
  printf("sigkill handler... suiciding\n");
  Task::current()->exit(Signal::SIGKILL);
}

void sigsegv_default_handler(int signal) {
  auto task = Task::current();

  printf("segmentation fault (pid: %d)\n", task->get_pid());
  task->exit(139);
}

void sig_unsupported_handler(int signal) {
  printk("this signal is not supported yet @_@\n");
}

}  // namespace valkyrie::kernel
