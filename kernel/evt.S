// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// evt.S - Exception Vector Table
//
// The table itself is required to be aligned to 0x800 (2048 i.e. 2^11),
// and each of its entries must be aligned to 0x80 (128 i.e. 2^7).

.macro evt_entry label
  .align 7
  b \label
.endm

.extern handle_exception

.section ".text"


.align 11
.global evt
evt:
  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception

  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception

  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception

  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception
  evt_entry handle_exception


.global evt_init
evt_init:
  // Load vbar_el2 with the address of exception vector table
  adr x0, evt
  msr vbar_el2, x0
  ret
