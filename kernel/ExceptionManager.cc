// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/ExceptionManager.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/TaskScheduler.h>

extern "C" void* evt;

namespace valkyrie::kernel {

bool ExceptionManager::_is_activated = false;

void ExceptionManager::handle_exception(TrapFrame* trap_frame) {
  uint64_t spsr_el1;
  asm volatile("mrs %0, SPSR_EL1" : "=r" (spsr_el1));

  const Exception ex = ExceptionManager::the().get_current_exception();

  // Issuing `svc #0` will trigger a switch from user mode to kernel mode,
  // where x8 is the system call id, and x0 ~ x5 are the arguments.
  if (ex.ec == 0b10101 && ex.iss == 0) [[likely]] {
    switch_user_va_space(nullptr);

    Task::current()->set_trap_frame(trap_frame);

    enableIRQs();

    // A process may return from `do_syscall()`,
    // e.g., a child task created by sys_fork(),
    // so we should call `Task::get_current()` again
    // to make sure that we are operating on the correct Task object.
    Task::current()->get_trap_frame()->x0 = do_syscall(trap_frame->x8,
                                                       trap_frame->x0,
                                                       trap_frame->x1,
                                                       trap_frame->x2,
                                                       trap_frame->x3,
                                                       trap_frame->x4,
                                                       trap_frame->x5);

    disableIRQs();

    // Handle pending POSIX signals.
    Task::current()->handle_pending_signals();

    TaskScheduler::the().maybe_reschedule();

    if (Task::current()->is_user_task()) {
      switch_user_va_space(Task::current()->get_ttbr0_el1());
    }

    return;
  }

  printk("Current exception lvl: %d\n", ExceptionManager::the().get_exception_level());
  printk("Saved Program Status Register: 0x%p\n", spsr_el1);
  printk("Exception return address: 0x%p\n", ex.ret_addr);
  printk("Exception class (EC): 0x%x\n", ex.ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", ex.iss);

  dump_registers(trap_frame);

  // For ec and iss, see ARMv8 manual p.1877
  switch (ex.ec) {
    case 0b011000:
      Kernel::panic("Trapped MSR, MRS, or System instruction execution\n");

    case 0b011001:
      Kernel::panic("Trapped access to SVE functionality\n");

    case 0b100000:
      Kernel::panic("Instruction Abort from a lower Exception Level\n");

    case 0b100001:
      Kernel::panic("Instruction Abort taken without a change in Exception level\n");

    case 0b100100:
      switch_user_va_space(nullptr);
      size_t fault_address;
      asm volatile("mrs %0, far_el1" : "=r"(fault_address));
      printf("segmentation fault (pid: %d)\n", Task::current()->get_pid());
      Task::current()->do_exit(4);
      switch_user_va_space(Task::current()->get_ttbr0_el1());
      break;

    case 0b100101:
      Kernel::panic("Data Abort taken without a change in Exception level (invalid data access)\n");

    default:
      Kernel::panic("Unknown exception: EC=%d, ISS=%d\n", ex.ec, ex.iss);
  }
}

void ExceptionManager::handle_irq(TrapFrame* trap_frame) {
  // XXX: For now we have tiemr IRQ as the only interrupt source.
  const Exception ex = ExceptionManager::the().get_current_exception();

  switch_user_va_space(nullptr);

  TimerMultiplexer::the().tick();
  TaskScheduler::the().tick();
  TaskScheduler::the().maybe_reschedule();

  // A timer IRQ can come from either kernel or user mode, so
  // we only need to restore ttbr0_el1 if we're returning to user mode.
  if (ex.ret_addr < KERNEL_BASE) {
    switch_user_va_space(Task::current()->get_ttbr0_el1());
  }
}


ExceptionManager::Exception ExceptionManager::get_current_exception() {
  Exception ex;
  uint32_t esr_el1; 

  asm volatile("mrs %0, ELR_EL1" : "=r" (ex.ret_addr));
  asm volatile("mrs %0, ESR_EL1" : "=r" (esr_el1));

  // ESR_EL1[31:26] = EC
  // ESR_EL1[25] = IL
  // ESR_EL1[24:0] = ISS
  ex.ec = esr_el1 >> 26;
  ex.iss = esr_el1 & 0x1ffffff;

  return ex;
}

void ExceptionManager::dump_registers(TrapFrame* trap_frame) {
  printk("--- Dumping CPU Registers ---\n");
  printk("x0: 0x%p\n", trap_frame->x0);
  printk("x1: 0x%p\n", trap_frame->x1);
  printk("x2: 0x%p\n", trap_frame->x2);
  printk("x3: 0x%p\n", trap_frame->x3);
  printk("x4: 0x%p\n", trap_frame->x4);
  printk("x30: 0x%p\n", trap_frame->x30);
  printk("SP_EL0: 0x%p\n", trap_frame->sp_el0);
  printk("ELR_EL1: 0x%p\n", trap_frame->elr_el1);
}

}  // namespace valkyrie::kernel
