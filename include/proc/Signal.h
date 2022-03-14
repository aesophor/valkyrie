// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// POSIX signals are a limited form of inter-process communication (IPC),
// typically used in Unix, Unix-like, and other POSIX-compliant operating systems.
//
// A signal is an asynchronous notification sent to a process or
// to a specific thread within the same process to notify it of an event.

#ifndef VALKYRIE_SIGNAL_H_
#define VALKYRIE_SIGNAL_H_

#include <Functional.h>

#include <dev/Console.h>

namespace valkyrie::kernel {

// clang-format off
enum Signal {
  SIGKILL = 9,
  SIGSEGV = 11,
  __NR_signals
};
// clang-format on

// Default signal handler table
using SignalHandler = void (*)(int);
extern SignalHandler __default_signal_handler_table[Signal::__NR_signals];

[[gnu::always_inline]] inline bool is_signal_valid(const int signal) {
  return signal >= 0 && signal < Signal::__NR_signals;
}

// Individual default signal handler declaration.
void sigkill_default_handler(int signal);
void sigsegv_default_handler(int signal);
void sig_unsupported_handler(int signal);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SIGNAL_H_
