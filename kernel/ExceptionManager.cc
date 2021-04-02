// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/ExceptionManager.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>

extern "C" void* evt;

namespace valkyrie::kernel {

ExceptionManager* ExceptionManager::get_instance() {
  static ExceptionManager instance;
  return &instance;
}

ExceptionManager::ExceptionManager() : _arm_core_timer() {
  // Install the address of exception vector table to VBAR_EL1.
  asm volatile("msr VBAR_EL1, %0" :: "r"(&evt));
}


void ExceptionManager::enable() {
  asm volatile("msr DAIFCLR, 0b1111");
}

void ExceptionManager::disable() {
  asm volatile("msr DAIFSET, 0b1111");
}


void ExceptionManager::handle_exception(const size_t number,
                                        const size_t arg1,
                                        const size_t arg2,
                                        const size_t arg3,
                                        const size_t arg4,
                                        const size_t arg5,
                                        const size_t arg6) {
  printk("system call = 0x%x\n", number);

  const Exception ex = get_current_exception();
  printk("Current exception lvl: %d\n", get_exception_level());
  printk("Exception return address: 0x%x\n", ex.ret_addr);
  printk("Exception class (EC): 0x%x\n", ex.ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", ex.iss);

  // For ec and iss, see ARMv8 manual p.1877
  switch (ex.ec) {
    case 0b10101:  // SVC instruction execution in AArch64 state
      if (ex.iss == 0) {
        syscall(number, arg1, arg2, arg3, arg4, arg5, arg6);
      }
      break;

    case 0b11000:
      Kernel::panic("Trapped MSR, MRS, or System instruction execution\n");
      break;

    case 0b011001:
      Kernel::panic("Trapped access to SVE functionality\n");
      break;

    default:
      Kernel::panic("Unknown exception: EC=%d, ISS=%d\n", ex.ec, ex.iss);
      break;
  }
}

void ExceptionManager::handle_irq() {
  _arm_core_timer.handle();
  _arm_core_timer.tick();
  printk("ARM core timer interrupt: jiffies = %d\n", _arm_core_timer.get_jiffies());
}


uint8_t ExceptionManager::get_exception_level() const {
  // Note: CurrentEL is Only accessible from EL1 or higher.
  uint8_t level;
  asm volatile("mrs %0, CurrentEL" : "=r" (level));
  return level >> 2;
}

void ExceptionManager::switch_to_exception_level(const uint8_t level,
                                                 const size_t new_sp) {
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
      spsr  = (1 << 0);       // use SP_ELx, not SP_EL0
      spsr |= (1 << 2);       // exception was taken from EL1
      spsr |= (0b1111 << 6);  // DAIF masked
      asm volatile("msr SPSR_EL2, %0" :: "r" (spsr));
      // Setup ELR_EL2
      asm volatile("msr ELR_EL2, %0" :: "r" (&&restore_link_register));
      break;

    case 0:
      // Setup EL0 stack
      asm volatile("msr SP_EL0, %0" :: "r" (saved_stack_pointer));
      // Setup SPSR_EL1 (Saved Processor Status Register)
      asm volatile("msr SPSR_EL1, %0" :: "r" (0));
      // Setup ELR_EL1
      asm volatile("msr ELR_EL1, %0" :: "r" (&&restore_link_register));
      break;

    default:
      return;
  }

  // Execute `eret`
  asm volatile("eret");

restore_link_register:
  // Restore `saved_return_address` to `lr`
  asm volatile("mov lr, %0" :: "r" (saved_return_address));

  // Maybe set the new stack
  if (new_sp) {
    asm volatile("mov sp, %0" :: "r" (new_sp));
  }
}


ARMCoreTimer& ExceptionManager::get_arm_core_timer() {
  return _arm_core_timer;
}

ExceptionManager::Exception ExceptionManager::get_current_exception() {
  Exception ex;

  // Obtain return address of current exception.
  asm volatile("mrs %0, elr_el1" : "=r" (ex.ret_addr));

  // esr_el2[31:26] = EC
  // esr_el2[25] = IL
  // esr_el2[24:0] = ISS
  uint32_t esr_el1; 
  asm volatile("mrs %0, esr_el1" : "=r" (esr_el1));
  ex.ec = esr_el1 >> 26;
  ex.iss = esr_el1 & 0x1ffffff;

  return ex;
}

}  // namespace valkyrie::kernel
