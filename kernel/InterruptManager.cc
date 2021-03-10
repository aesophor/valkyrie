// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <InterruptManager.h>

#include <Console.h>

extern "C" void evt_init(void);
extern "C" uint32_t get_el(void);
extern "C" uint32_t get_esr_el2(void);
extern "C" size_t get_elr_el2(void);

namespace valkyrie::kernel {

InterruptManager::InterruptManager()
    : _current_exception_level(get_el()) {
  evt_init();
}


void InterruptManager::enable() {

}

void InterruptManager::disable() {

}

void InterruptManager::handle_irq() {
  uint32_t irq = io::get<uint32_t>(IRQ_PENDING_1);

  // esr_el2[31:26] = EC
  // esr_el2[25] = IL
  // esr_el2[24:0] = ISS
  size_t ret_addr = get_elr_el2();
  uint32_t esr_el2 = get_esr_el2();
  uint8_t ec = esr_el2 >> 26;
  uint32_t iss = esr_el2 & 0x1ffffff;

  printk("Exception return address: 0x%x\n", ret_addr);
  printk("Exception class (EC): 0x%x\n", ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", iss);

  /*
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):
      puts("received timer irq");
      break;
    default:
      puts("Unknown pending irq...");
      break;
  }
  */
}


uint8_t InterruptManager::get_current_exception_level() const {
  return _current_exception_level;
}

}  // namespace valkyrie::kernel
