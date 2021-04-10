// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Compiler.h>

#include <kernel/Kernel.h>

using valkyrie::kernel::Kernel;

extern "C" void __cxa_pure_virtual(void) {
  Kernel::panic("__cxa_pure_virtual invoked\n");
}
