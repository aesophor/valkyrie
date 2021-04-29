// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// Signals are a limited form of inter-process communication (IPC),
// typically used in Unix, Unix-like, and other POSIX-compliant
// operating systems.
//
// A signal is an asynchronous notification sent to a process or
// to a specific thread within the same process to notify it of an event.

#ifndef VALKYRIE_SIGNAL_H_
#define VALKYRIE_SIGNAL_H_

#include <Functional.h>

namespace valkyrie::kernel {

enum Signal {
  SIGINT = 2,
  SIGKILL = 9,
  __NR_signals
};

// Default signal handler table
extern void (*__default_signal_handler_table[Signal::__NR_signals])();

// Individual default signal handler declaration.
void sigint_default_handler();
void sigkill_default_handler();
void sig_unsupported_handler();


inline bool is_signal_valid(const int signal) {
  return signal >= 0 && signal < Signal::__NR_signals;
}

inline void invoke_default_signal_handler(const Signal signal) {
  if (!is_signal_valid(signal)) {
    printk("do_signal_handler: bad signal: (id=0x%x)\n", signal);
    return;
  }

  __default_signal_handler_table[signal]();
}

}  // namespace valkyrie::kernl

#endif  // VALKYRIE_SIGNAL_H_
