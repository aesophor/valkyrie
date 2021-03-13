// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <ExceptionManager.h>

#include <Console.h>
#include <Kernel.h>

#define EL0_STACK 0x20000
#define EL1_STACK 0x40000

extern "C" void* evt;

namespace valkyrie::kernel {

ExceptionManager* ExceptionManager::get_instance() {
  static ExceptionManager instance;
  return &instance;
}

ExceptionManager::ExceptionManager()
    : _exception_level(),
      _arm_core_timer() {
  // Install the address of exception vector table to VBAR_EL1.
  asm volatile("msr VBAR_EL1, %0" :: "r"(&evt));
}


void ExceptionManager::enable_irqs() {
  asm volatile("msr daifclr, 0b0010");  // unmask IRQs
}

void ExceptionManager::disable_irqs() {
  asm volatile("msr daifset, 0b0010");  // mask IRQs
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
  printk("Current exception lvl: %d\n", get_exception_level());
}

void ExceptionManager::handle_irq() {
  _arm_core_timer.handle();
  _arm_core_timer.tick();

  printk("ARM core timer interrupt: jiffies = %d\n", _arm_core_timer.get_jiffies());
}


uint8_t ExceptionManager::get_exception_level() const {
  // Only accessible from EL1 or higher.
  if (_exception_level == 0) {
    return 0;
  }

  uint8_t level;
  asm volatile("mrs %0, CurrentEL" : "=r" (level));
  return level >> 2;
}

void ExceptionManager::switch_to_exception_level(const uint8_t level) {
  uint64_t spsr;
  void* saved_stack_pointer;
  void* saved_return_address;

  asm volatile("mov %0, lr" : "=r" (saved_return_address));
  asm volatile("mov %0, sp" : "=r" (saved_stack_pointer));

  switch (level) {
    case 1:
      // Setup EL1 stack
      asm volatile("msr SP_EL1, %0" :: "r" (saved_stack_pointer));
      // Setup SPSR_EL2 (Saved Processor Status Register)
      asm volatile("mrs %0, SPSR_EL2" : "=r" (spsr));
      spsr |= (1 << 0);  // use SP_ELx, not SP_EL0
      spsr |= (1 << 2);  // exception was taken from EL1
      spsr |= (0b1111 << 6);  // DAIF masked
      asm volatile("msr SPSR_EL2, %0" :: "r" (spsr));
      // Setup ELR_EL2
      asm volatile("msr ELR_EL2, %0" :: "r" (&&__restore_link_register));
      break;

    case 0:
      // Setup EL0 stack (FIXME: define USER_STACK)
      asm volatile("msr SP_EL0, %0" :: "r" (saved_stack_pointer));
      // Setup SPSR_EL1 (Saved Processor Status Register)
      asm volatile("msr SPSR_EL1, %0" :: "r" (0));
      // Setup ELR_EL1
      asm volatile("msr ELR_EL1, %0" :: "r" (&&__restore_link_register));
      break;

    default:
      break;
  }

  _exception_level = level;

  // Execute `eret`
  asm volatile("eret");

__restore_link_register:
  // Restore `saved_return_address` to `lr`
  asm volatile("mov lr, %0" :: "r" (saved_return_address));
}


ARMCoreTimer& ExceptionManager::get_arm_core_timer() {
  return _arm_core_timer;
}

}  // namespace valkyrie::kernel
