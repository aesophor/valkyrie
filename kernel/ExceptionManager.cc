// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/ExceptionManager.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <kernel/TimerMultiplexer.h>

extern "C" void* evt;

namespace valkyrie::kernel {

ExceptionManager& ExceptionManager::get_instance() {
  static ExceptionManager instance;
  return instance;
}

ExceptionManager::ExceptionManager()
    : _tasklet_scheduler() {
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

  uint64_t spsr_el1;
  asm volatile("mrs %0, SPSR_EL1" : "=r"(spsr_el1));

  const Exception ex = get_current_exception();
  printk("Current exception lvl: %d\n", get_exception_level());
  printk("Saved Program Status Register: 0x%x\n", spsr_el1);
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

    case 0b011001:
      Kernel::panic("Trapped access to SVE functionality\n");

    case 0b100101:
      Kernel::panic("Data Abort taken without a change in Exception level (invalid data access)\n");

    default:
      Kernel::panic("Unknown exception: EC=%d, ISS=%d\n", ex.ec, ex.iss);
  }
}

void ExceptionManager::handle_irq() {
  if (MiniUART::get_instance().has_pending_irq()) {
    MiniUART::get_instance().handle_irq();
  } else {
    TimerMultiplexer::get_instance().tick();

    auto task = []() { printf("ok\n"); };
    _tasklet_scheduler.add_tasklet(task);

    // Do all the unfinished deferred tasks
    // with interrupts enabled.
    enable();
    _tasklet_scheduler.finish_all();
  }
}


uint8_t ExceptionManager::get_exception_level() const {
  // Note: CurrentEL is Only accessible from EL1 or higher.
  uint8_t level;
  asm volatile("mrs %0, CurrentEL" : "=r" (level));
  return level >> 2;
}

void ExceptionManager::switch_to_exception_level(const uint8_t level,
                                                 void* ret_addr,
                                                 void* new_sp) {
  uint64_t spsr;
  void* return_address = (ret_addr) ? ret_addr : &&out;
  void* stack_pointer = (new_sp) ? new_sp : nullptr;

  // If the user hasn't specified `new_sp` (which means it is nullptr),
  // then we will use the current SP as the new SP after `eret`.
  if (!stack_pointer) {
    asm volatile("mov %0, sp" : "=r" (stack_pointer));
  }

  switch (level) {
    case 1:
      // Setup EL1 stack
      asm volatile("msr SP_EL1, %0" :: "r" (stack_pointer));
      // Setup SPSR_EL2 (Saved Processor Status Register)
      spsr  = (1 << 0);       // use SP_ELx, not SP_EL0
      spsr |= (1 << 2);       // exception was taken from EL1
      spsr |= (0b1111 << 6);  // DAIF masked
      asm volatile("msr SPSR_EL2, %0" :: "r" (spsr));
      // Setup ELR_EL2
      asm volatile("msr ELR_EL2, %0" :: "r" (return_address));
      break;

    case 0:
      // Setup EL0 stack
      asm volatile("msr SP_EL0, %0" :: "r" (stack_pointer));
      // Setup SPSR_EL1 (Saved Processor Status Register)
      asm volatile("msr SPSR_EL1, %0" :: "r" (0x3c0));
      // Setup ELR_EL1
      asm volatile("msr ELR_EL1, %0" :: "r" (return_address));
      break;

    default:
      return;
  }

  // Execute `eret`
  asm volatile("eret");

out:
  // Do nothing.
  ;
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
