// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <proc/Signal.h>

namespace valkyrie::kernel {

Signal::Signal(Signal::Type number, void (*handler)())
    : _number(number),
      _handler(handler) {}

}  // namespace valkyrie::kernel
