// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/ExceptionManager.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/TaskScheduler.h>

extern "C" void* evt;

namespace valkyrie::kernel {

ExceptionManager& ExceptionManager::get_instance() {
  static ExceptionManager instance;
  return instance;
}

ExceptionManager::ExceptionManager()
    : _is_enabled(),
      _tasklet_scheduler() {
  // Install the address of exception vector table to VBAR_EL1.
  asm volatile("msr VBAR_EL1, %0" :: "r"(&evt));
}


void ExceptionManager::handle_exception(TrapFrame* trap_frame) {
  uint64_t spsr_el1;
  asm volatile("mrs %0, SPSR_EL1" : "=r" (spsr_el1));

  const Exception ex = ExceptionManager::get_instance().get_current_exception();

  // Issuing `svc #0` will trigger a switch from user mode to kernel mode,
  // where x8 is the system call id, and x0 ~ x5 are the arguments.
  if (likely(ex.ec == 0b10101 && ex.iss == 0)) {
    Task::current()->set_trap_frame(trap_frame);

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

    // Handle pending POSIX signals.
    Task::current()->handle_pending_signals();

    // User preemption.
    TaskScheduler::get_instance().maybe_reschedule();
    return;
  }

  printk("Current exception lvl: %d\n", ExceptionManager::get_instance().get_exception_level());
  printk("Saved Program Status Register: 0x%x\n", spsr_el1);
  printk("Exception return address: 0x%x\n", ex.ret_addr);
  printk("Exception class (EC): 0x%x\n", ex.ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", ex.iss);

  // For ec and iss, see ARMv8 manual p.1877
  switch (ex.ec) {
    case 0b11000:
      Kernel::panic("Trapped MSR, MRS, or System instruction execution\n");

    case 0b011001:
      Kernel::panic("Trapped access to SVE functionality\n");

    case 0b100001:
      Kernel::panic("Instruction Abort taken without a change in Exception level.\n");

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

    auto& sched = TaskScheduler::get_instance();
    sched.tick();
    sched.maybe_reschedule();

    /*
    auto task = []() { printf("ok\n"); };
    _tasklet_scheduler.add_tasklet(task);

    // Do all the unfinished deferred tasks
    // with interrupts enabled.
    enable();
    _tasklet_scheduler.finish_all();
    */
  }
}


uint8_t ExceptionManager::get_exception_level() const {
  // Note: CurrentEL is Only accessible from EL1 or higher.
  uint8_t level;
  asm volatile("mrs %0, CurrentEL" : "=r" (level));
  return level >> 2;
}

void ExceptionManager::downgrade_exception_level(const uint8_t level,
                                                 void* ret_addr,
                                                 void* high_level_sp,
                                                 void* low_level_sp) {
  // If the user hasn't specified `ret_addr` (which means it is nullptr),
  // then we will use the address out `out` as the new PC after `eret`
  // so that the program will keep executing like a normal function.
  void* return_address = (ret_addr) ? ret_addr : &&out;

  // If the user hasn't specified `low_level_sp` (which means it is nullptr),
  // then we will use the current SP as the new SP after `eret`.
  if (!low_level_sp) {
    asm volatile("mov %0, sp" : "=r"(low_level_sp));
  }

  switch (level) {
    case 1:
      asm volatile("msr SP_EL1, %0" :: "r"(low_level_sp));
      uint64_t spsr;
      spsr  = (1 << 0);       // use SP_ELx, not SP_EL0
      spsr |= (1 << 2);       // exception was taken from EL1
      spsr |= (0b1111 << 6);  // DAIF masked
      asm volatile("msr SPSR_EL2, %0" :: "r"(spsr));
      asm volatile("msr ELR_EL2, %0" :: "r"(return_address));
      break;

    case 0:
      asm volatile("msr SP_EL0, %0" :: "r"(low_level_sp));
      asm volatile("msr SPSR_EL1, %0" :: "r"(0));
      asm volatile("msr ELR_EL1, %0" :: "r"(return_address));
      break;

    default:
      goto out;
  }

  // Each user task will have a kernel stack and a user stack.
  // The value of SP before `eret` will be the SP after the user task
  // switches from EL0 to EL1.
  if (high_level_sp) {
    asm volatile("mov SP, %0" :: "r"(high_level_sp));
  }

  // Finally downgrade the exception level.
  asm volatile("eret");

out:
  return;
}

bool ExceptionManager::is_enabled() const {
  return _is_enabled;
}


ExceptionManager::Exception ExceptionManager::get_current_exception() {
  Exception ex;

  // Obtain return address of current exception.
  asm volatile("mrs %0, ELR_EL1" : "=r" (ex.ret_addr));

  // ESR_EL1[31:26] = EC
  // ESR_EL1[25] = IL
  // ESR_EL1[24:0] = ISS
  uint32_t esr_el1; 
  asm volatile("mrs %0, ESR_EL1" : "=r" (esr_el1));
  ex.ec = esr_el1 >> 26;
  ex.iss = esr_el1 & 0x1ffffff;

  return ex;
}

}  // namespace valkyrie::kernel
