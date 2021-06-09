// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

#include <libs/CString.h>

extern uint8_t* _bss_start;
extern uint8_t* _bss_end;

using ctor_func_t = void (*)();
extern ctor_func_t start_ctors;
extern ctor_func_t end_ctors;


extern "C" [[noreturn]] void kmain(void) {
  // Initialize .bss
  memset(_bss_start, 0, _bss_end - _bss_start);

  // Invoke all static global constructors.
  for (ctor_func_t* ctor = &start_ctors; ctor != &end_ctors; ctor++) {
    (*ctor)();
  }

  // Valkyrie Kernel C++ entry point  ヽ(○´∀`○)ﾉ
  valkyrie::kernel::Kernel::get_instance().run();
}
