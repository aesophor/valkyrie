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

class Signal {
 public:
  enum Type {
    SIGHUP = 1,
    SIGINT,
    SIGQUIT,
    SIGFPE = 8,
    SIGKILL,
    SIGALRM,
    SIGTERM,
    __NR_signals
  };

  Signal(Signal::Type number, void (*handler)());
  ~Signal() = default;

 private:
  Signal::Type _number;
  void (*_handler)();
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SIGNAL_H_
