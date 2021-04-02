// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

using ctor_func_t = void (*)();
extern ctor_func_t start_ctors;
extern ctor_func_t end_ctors;

// Valkyrie Kernel C++ entry point  ヽ(○´∀`○)ﾉ
extern "C" [[noreturn]] void kmain(void) {
  // Invoke all static global constructors.
  for (ctor_func_t* ctor = &start_ctors; ctor != &end_ctors; ctor++) {
    (*ctor)();
  }

  valkyrie::kernel::Kernel::get_instance()->run();
}
