// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <InterruptManager.h>

#include <Console.h>

extern "C" {

void evt_init(void);
uint32_t get_el(void);
uint32_t get_esr_el2(void);
size_t get_elr_el2(void);
void enable_irq(void);
void enable_el2_irq(void);

}


namespace valkyrie::kernel {

InterruptManager::InterruptManager()
    : _current_exception_level(get_el()),
      _arm_core_timer() {
  evt_init();

  // To use interrupt in EL2, you need to
  // 1. set HCR_EL2.IMO
  // 2. clear PSTATE.DAIF
  // in order to enable CPU to accept interrupt.
  enable_el2_irq();
}


void InterruptManager::enable() {
  _arm_core_timer.enable();
  enable_irq();
}

void InterruptManager::disable() {

}

void InterruptManager::handle_irq() {
  /*
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

  _arm_core_timer.handle();
  */

  static uint32_t jiffie = 0;
  printk("ARM core timer interrupt: jiffie = %d\n", jiffie);
  _arm_core_timer.handle();
  ++jiffie;
}


uint8_t InterruptManager::get_current_exception_level() const {
  return _current_exception_level;
}

}  // namespace valkyrie::kernel
