// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

#include <CString.h>

extern char _bss_start[0];
extern char _bss_end[0];

using ctor_func_t = void (*)();
extern ctor_func_t start_ctors;
extern ctor_func_t end_ctors;

using namespace valkyrie::kernel;

extern "C" [[noreturn]] void kmain(void) {
  memset(_bss_start, 0, _bss_end - _bss_start);

  for (ctor_func_t *ctor = &start_ctors; ctor != &end_ctors; ctor++) {
    (*ctor)();
  }

  Kernel::the().run();
}
