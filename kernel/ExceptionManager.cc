// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <ExceptionManager.h>

#include <Console.h>

#define HCR_EL2_RW (1 << 31)

namespace valkyrie::kernel {

ExceptionManager* ExceptionManager::get_instance() {
  static ExceptionManager instance;
  return &instance;
}

ExceptionManager::ExceptionManager() : _arm_core_timer() {}


void ExceptionManager::enable() {
  // Unmask IRQs
  asm volatile("msr daifclr, 0b0010");
}

void ExceptionManager::disable() {
  // Mask IRQs
  asm volatile("msr daifset, 0b0010");
}


void ExceptionManager::handle_exception() {
  // esr_el2[31:26] = EC
  // esr_el2[25] = IL
  // esr_el2[24:0] = ISS
  size_t ret_addr;
  uint32_t esr_el1; 

  asm volatile("mrs %0, elr_el1" : "=r" (ret_addr));
  asm volatile("mrs %0, esr_el1" : "=r" (esr_el1));

  uint8_t ec = esr_el1 >> 26;
  uint32_t iss = esr_el1 & 0x1ffffff;

  printk("Exception return address: 0x%x\n", ret_addr);
  printk("Exception class (EC): 0x%x\n", ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", iss);
}

void ExceptionManager::handle_irq() {
  _arm_core_timer.handle();
  _arm_core_timer.tick();

  printk("ARM core timer interrupt: jiffies = %d\n", _arm_core_timer.get_jiffies());
}


uint8_t ExceptionManager::get_exception_level() const {
  // Only accessible from EL1 or higher.
//  uint8_t level;
//  asm volatile("mrs %0, CurrentEL" : "=r" (level));
//  return level >> 2;
  return 0;
}

void ExceptionManager::set_exception_level(const uint8_t level) const {
  // Switch from EL2 to EL1.

  // Set HCR_EL2.RW to 1 (which is required for 64-bit kernels)

}


ARMCoreTimer& ExceptionManager::get_arm_core_timer() {
  return _arm_core_timer;
}

}  // namespace valkyrie::kernel
