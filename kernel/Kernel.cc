// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Kernel.h>

#include <Deque.h>
#include <Functional.h>
#include <Memory.h>
#include <String.h>
#include <dev/Console.h>
#include <fs/ELF.h>
#include <proc/Scheduler.h>
#include <usr/Shell.h>

extern "C" [[noreturn]] void _halt(void);

namespace valkyrie::kernel {

class Base {
 public:
  Base() { printf("Base::Base()\n"); }
  virtual ~Base() { printf("Base::~Base()\n"); }

  virtual void func() { printf("Base::func()\n"); }
};

class Derived : public Base {
 public:
  Derived() : Base() { printf("Derived::Derived()\n"); }
  virtual ~Derived() { printf("Derived::~Derived()\n"); }

  virtual void func() override { printf("Derived::func()\n"); }
};


Kernel* Kernel::get_instance() {
  static Kernel instance;
  return &instance;
}

Kernel::Kernel()
    : _exception_manager(ExceptionManager::get_instance()),
      _memory_manager(MemoryManager::get_instance()),
      _mini_uart(MiniUART::get_instance()),
      _timer_multiplexer(TimerMultiplexer::get_instance()),
      _mailbox(Mailbox::get_instance()),
      _initramfs() {}


void Kernel::run() {
  //_mini_uart.set_read_buffer_enabled(true);
  _mini_uart.set_write_buffer_enabled(true);

  print_banner();
  print_hardware_info();

  printk("switching to supervisor mode... (≧▽ ≦)\n");
  _exception_manager.switch_to_exception_level(1);
  _exception_manager.enable();

 
  Function<void ()> fout;
  {
    String s = "hi";

    Function<void ()> f = [s]() { printf("%s\n", s.c_str()); };
    f();

    fout = f;
  }
  fout();


  {
    Deque<int> v1;
    Deque<int> v2;

    v1.push_back(3);
    v1.push_back(5);
    v1.push_back(7);

    v2 = move(v1);

    printf("v1 = [");
    for (int i = 0; i < (int) v1.size(); i++) {
      printf("%d ", v1[i]);
    }
    printf("]\n");

    printf("v2 = [");
    for (int i = 0; i < (int) v2.size(); i++) {
      printf("%d ", v2[i]);
    }
    printf("]\n");
  }


  /*
  {
    String s1 = "fuck";
    String s2 = "wow";

    s1 = s1 + s2 + s1 + s2 + s1 + "omg";
    s2 = s1;
    //s2 = move(s1);

    //String s2 = s1 + "omg";
    //String s2 = move(s1);
    //s2[1] = 'a';
    printf("s1 (0x%x) = %s\n", s1.c_str(), s1.c_str());
    printf("s2 (0x%x) = %s\n", s2.c_str(), s2.c_str());
  }
  */

  //printk("switching to user mode... (≧▽ ≦)\n");
  //_exception_manager.switch_to_exception_level(0, /*new_sp=*/0x20000);

  // Lab1 SimpleShell
  //auto shell = make_shared<Shell>();
  Shell().run();

  printf("you shouldn't have reached here :(\n");
  _halt();
}


void Kernel::print_banner() {
  console::set_color(console::Color::GREEN, /*bold=*/true);
  puts("--- Valkyrie OS ---");
  console::set_color(console::Color::YELLOW, /*bold=*/true);
  puts("Developed by: Marco Wang <aesophor.cs09g@nctu.edu.tw>");
  console::clear_color();
}

void Kernel::print_hardware_info() {
  const auto board_revision = _mailbox.get_board_revision();
  const auto vc_memory_info = _mailbox.get_vc_memory();

  printk("board revision: 0x%x\n", board_revision);
  printk("VC core base address: 0x%x\n", vc_memory_info.first);
  printk("VC core size: 0x%x\n", vc_memory_info.second);
}


Initramfs& Kernel::get_initramfs() {
  return _initramfs;
}

}  // namespace valkyrie::kernel
