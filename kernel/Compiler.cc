// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// Compiler.cc - Some routines required by the linker.
// 
// References:
// [1] https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor-runtime-api

#include <kernel/Kernel.h>

using valkyrie::kernel::Kernel;

extern "C" void __cxa_pure_virtual(void) {
  Kernel::panic("__cxa_pure_virtual invoked (likely caused by use-after-free)\n");
}

extern "C" void __cxa_atexit(void) {}

// __dso_handle is a "guard" that is used to
// identify dynamic shared objects during global destruction.
void* __dso_handle = nullptr;
