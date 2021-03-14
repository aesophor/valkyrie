// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <Kernel.h>

#include <Console.h>

using ctor_func_t = void (*)();
extern ctor_func_t start_ctors;
extern ctor_func_t end_ctors;

// Valkyrie Kernel C++ entry point  ヽ(○´∀`○)ﾉ
extern "C" [[noreturn]] void kmain(void) {
  // Invoke all static global constructors in the kernel.
  for (ctor_func_t* ctor = &start_ctors; ctor != &end_ctors; ctor++) {
    (*ctor)();
  }

  // Run the kernel.
  valkyrie::kernel::Kernel::get_instance()->run();

  // We should never reach here.
  while (1);
}
