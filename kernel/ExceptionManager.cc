// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <ExceptionManager.h>

#include <Console.h>

namespace valkyrie::kernel {

ExceptionManager* ExceptionManager::get_instance() {
  static ExceptionManager instance;
  return &instance;
}

ExceptionManager::ExceptionManager()
    : _arm_core_timer() {
  // Install the address of exception vector table to vbar_el2.
  asm volatile(
      "adr x0, evt      \n\t\
       msr vbar_el2, x0 ");

  // To use interrupt in EL2, you need to
  // 1. set HCR_EL2.IMO
  // 2. clear PSTATE.DAIF
  // in order to enable CPU to accept interrupt.
  asm volatile(
      "mrs x0, hcr_el2     \n\t\
       orr x0, x0, 0b10000 \n\t\
       msr hcr_el2, x0     \n\t\
       msr daifclr, 0b1111 ");
}


void ExceptionManager::enable() {
  asm volatile("msr daifclr, 0b0010");
}

void ExceptionManager::disable() {
  asm volatile("msr daifset, 0b0010");
}


void ExceptionManager::handle_exception() {
  // esr_el2[31:26] = EC
  // esr_el2[25] = IL
  // esr_el2[24:0] = ISS
  size_t ret_addr;
  uint32_t esr_el2; 

  asm volatile("mrs %0, elr_el2" : "=r" (ret_addr));
  asm volatile("mrs %0, esr_el2" : "=r" (esr_el2));

  uint8_t ec = esr_el2 >> 26;
  uint32_t iss = esr_el2 & 0x1ffffff;

  printk("Exception return address: 0x%x\n", ret_addr);
  printk("Exception class (EC): 0x%x\n", ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", iss);
}

void ExceptionManager::handle_irq() {
  _arm_core_timer.handle();
  _arm_core_timer.tick();

  printk("ARM core timer interrupt: jiffies = %d\n", _arm_core_timer.get_jiffies());
}


uint8_t ExceptionManager::get_current_exception_level() const {
  uint8_t exception_level;
  asm volatile("mrs %0, CurrentEL" : "=r" (exception_level));
  return exception_level >> 2;
}

ARMCoreTimer& ExceptionManager::get_arm_core_timer() {
  return _arm_core_timer;
}

}  // namespace valkyrie::kernel
