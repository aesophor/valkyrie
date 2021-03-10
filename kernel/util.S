.section ".text"
.global omg
omg:
  svc 1
  ret

.global get_el
get_el:
  mrs x0, CurrentEL
  lsr x0, x0, 2
  ret

.global get_esr_el2
get_esr_el2:
  mrs x0, esr_el2
  ret

.global get_elr_el2
get_elr_el2:
  mrs x0, elr_el2
  ret

.global get_cntfrq_el0
get_cntfrq_el0:
  mrs x0, cntfrq_el0
  ret

.global get_cntpct_el0
get_cntpct_el0:
  mrs x0, cntpct_el0
  ret

.global enable_irq
enable_irq:
  msr daifclr, 2
  ret

.global disable_irq
disable_irq:
  msr daifset, 2
  ret

.global enable_el2_irq
enable_el2_irq:
  // 1. Set HCR_EL2.IMO
  mrs x0, hcr_el2
  orr x0, x0, 0b10000
  msr hcr_el2, x0
  // 2. clear PSTATE.DAIF
  msr daifclr, 0b1111
  ret
