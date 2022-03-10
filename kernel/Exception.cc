// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <kernel/Exception.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <kernel/Syscall.h>
#include <kernel/TimerMultiplexer.h>
#include <proc/TaskScheduler.h>

namespace valkyrie::kernel::exception {

bool _is_activated = false;

struct Exception {
  uint8_t ec;     // exception class
  uint8_t level;  // exception level
  uint32_t iss;   // instruction specific syndrome
  uint64_t ret_addr;
  uint64_t spsr_el1;
};

Exception get_current_exception() {
  Exception ex;
  uint32_t esr_el1;

  asm volatile("mrs %0, ELR_EL1" : "=r"(ex.ret_addr));
  asm volatile("mrs %0, SPSR_EL1" : "=r"(ex.spsr_el1));
  asm volatile("mrs %0, CurrentEL" : "=r"(ex.level));
  asm volatile("mrs %0, ESR_EL1" : "=r"(esr_el1));

  // ESR_EL1[31:26] = EC
  // ESR_EL1[25] = IL
  // ESR_EL1[24:0] = ISS
  ex.ec = esr_el1 >> 26;
  ex.iss = esr_el1 & 0x1ffffff;
  ex.level >>= 2;

  return ex;
}

uint64_t get_fault_page_addr() {
  size_t fault_address;
  asm volatile("mrs %0, far_el1" : "=r"(fault_address));
  return fault_address & PAGE_MASK;
}

void handle_syscall(TrapFrame *trap_frame) {
  switch_user_va_space(nullptr);

  Task::current()->set_trap_frame(trap_frame);

  enable_irqs();

  // A process may return from `do_syscall()`, e.g., a child task created by
  // sys_fork(), so we should call `Task::get_current()` again to make sure
  // that we are operating on the correct Task object.
  // clang-format off
  Task::current()->get_trap_frame()->x0 = do_syscall(trap_frame->x8,
                                                     trap_frame->x0,
                                                     trap_frame->x1,
                                                     trap_frame->x2,
                                                     trap_frame->x3,
                                                     trap_frame->x4,
                                                     trap_frame->x5);
  // clang-format on

  disable_irqs();

  // Handle pending POSIX signals.
  Task::current()->handle_pending_signals();

  // If the current task has used up its time slice, preempt it with the next one.
  TaskScheduler::the().maybe_schedule();

  if (Task::current()->is_user_task()) {
    switch_user_va_space(Task::current()->get_ttbr0_el1());
  }
}

void handle_page_fault() {
  switch_user_va_space(nullptr);

  size_t fault_page_addr = get_fault_page_addr();

  if (Task::current()->get_vmmap().is_cow_page(fault_page_addr)) {
    // Writing to a Copy-on-Write page, copy page frame and update PTE for current task.
    Task::current()->get_vmmap().copy_page_frame(fault_page_addr);
  } else {
    // Permission fault (trying to write to a read-only page)
    printf("segmentation fault (pid: %d)\n", Task::current()->get_pid());
    Task::current()->exit(4);
  }

  switch_user_va_space(Task::current()->get_ttbr0_el1());
}

void unhandled_exception(const Exception &ex) {
  printk("Current exception lvl: %d\n", ex.level);
  printk("Saved Program Status Register: 0x%p\n", ex.spsr_el1);
  printk("Exception return address: 0x%p\n", ex.ret_addr);
  printk("Exception class (EC): 0x%x\n", ex.ec);
  printk("Instruction specific syndrome (ISS): 0x%x\n", ex.iss);

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
    case 0b100101:
      Kernel::panic(
          "Data Abort taken without a change in Exception level (invalid data access)\n");
    default:
      Kernel::panic("Unknown exception: EC=%d, ISS=%d\n", ex.ec, ex.iss);
  }
}

void handle_exception(TrapFrame *trap_frame) {
  const Exception ex = get_current_exception();

  // Issuing `svc #0` will trigger a switch from user mode to kernel mode,
  // where x8 is the system call id, and x0 ~ x5 are the arguments.
  if (ex.ec == 0b10101 && ex.iss == 0) {
    handle_syscall(trap_frame);
  } else if (ex.ec == 0b100100) {
    handle_page_fault();
  } else {
    unhandled_exception(ex);
  }
}

void handle_irq(TrapFrame *trap_frame) {
  // XXX: For now we have tiemr IRQ as the only interrupt source.
  const Exception ex = get_current_exception();

  switch_user_va_space(nullptr);

  TimerMultiplexer::the().tick();
  TaskScheduler::the().tick();
  TaskScheduler::the().maybe_schedule();

  // A timer IRQ can come from either kernel or user mode, so
  // we only need to restore ttbr0_el1 if we're returning to user mode.
  if (ex.ret_addr < KERNEL_VA_BASE) {
    switch_user_va_space(Task::current()->get_ttbr0_el1());
  }
}

}  // namespace valkyrie::kernel::exception
