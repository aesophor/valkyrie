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
