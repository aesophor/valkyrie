// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_GPIO_H_
#define VALKYRIE_GPIO_H_

#include <driver/IO.h>

// Defined according to BCM2837-ARM-Peripherals.pdf (pg. 90 ~ 91)
// 刻到手軟 Zzzzz
#define GPFSEL0   (MMIO_BASE + 0x200000)
#define GPFSEL1   (MMIO_BASE + 0x200004)
#define GPFSEL2   (MMIO_BASE + 0x200008)
#define GPFSEL3   (MMIO_BASE + 0x20000c)
#define GPFSEL4   (MMIO_BASE + 0x200010)
#define GPFSEL5   (MMIO_BASE + 0x200014)
#define GPSET0    (MMIO_BASE + 0x20001c)
#define GPSET1    (MMIO_BASE + 0x200020)
#define GPCLR0    (MMIO_BASE + 0x200028)
#define GPCLR1    (MMIO_BASE + 0x20002c)
#define GPLEV0    (MMIO_BASE + 0x200034)
#define GPLEV1    (MMIO_BASE + 0x200038)
#define GPEDS0    (MMIO_BASE + 0x200040)
#define GPEDS1    (MMIO_BASE + 0x200044)
#define GPREN0    (MMIO_BASE + 0x20004c)
#define GPREN1    (MMIO_BASE + 0x200050)
#define GPFEN0    (MMIO_BASE + 0x200058)
#define GPFEN1    (MMIO_BASE + 0x20005c)
#define GPHEN0    (MMIO_BASE + 0x200064)
#define GPHEN1    (MMIO_BASE + 0x200068)
#define GPLEN0    (MMIO_BASE + 0x200070)
#define GPLEN1    (MMIO_BASE + 0x200074)
#define GPAREN0   (MMIO_BASE + 0x20007c)
#define GPAREN1   (MMIO_BASE + 0x200080)
#define GPAFEN0   (MMIO_BASE + 0x200088)
#define GPAFEN1   (MMIO_BASE + 0x20008c)
#define GPPUD     (MMIO_BASE + 0x200094)
#define GPPUDCLK0 (MMIO_BASE + 0x200098)
#define GPPUDCLK1 (MMIO_BASE + 0x20009c)

// PUD (Pull-Up/Down) modes
#define PUD_DISABLED          0b00
#define PUD_PULL_DOWN_ENABLED 0b01
#define PUD_PULL_UP_ENABLED   0b10

#endif  // VALKYRIE_GPIO_H_
