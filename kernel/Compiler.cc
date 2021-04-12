// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Compiler.h>

#include <kernel/Kernel.h>

using valkyrie::kernel::Kernel;

extern "C" void __cxa_pure_virtual(void) {
  Kernel::panic("__cxa_pure_virtual invoked (possibly due to use-after-free)\n");
}

extern "C" void __cxa_atexit(void) {

}

extern "C" void __dso_handle(void) {
  Kernel::panic("__dso_handle invoked\n");
}
