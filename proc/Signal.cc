// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Signal.h>

#include <proc/Task.h>

namespace valkyrie::kernel {

#define UNDEFINED sig_unsupported_handler

void (*__default_signal_handler_table[Signal::__NR_signals])() = {
  UNDEFINED,
  UNDEFINED,
  sigint_default_handler,
  UNDEFINED,
  UNDEFINED,
  UNDEFINED,
  UNDEFINED,
  UNDEFINED,
  UNDEFINED,
  sigkill_default_handler
};


void sigint_default_handler() {
  sig_unsupported_handler();
}

void sigkill_default_handler() {
  printk("sigkill handler... suiciding\n");
  Task::current()->do_exit(Signal::SIGKILL);
}

void sig_unsupported_handler() {
  printk("this signal is not supported yet @_@\n");
}

}  // namespace valkyrie::kernel
