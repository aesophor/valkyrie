# Copyright (c) 2021-2022 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

UNAME_S := $(shell uname -s)
$(info UNAME_S=$(UNAME_S))

UNAME_M := $(shell uname -m)
$(info UNAME_M=$(UNAME_M))

ifeq ($(UNAME_S)$(UNAME_M),Darwinarm64)
	TOOLCHAIN_PREFIX = aarch64-elf-
else ifeq ($(UNAME_S)$(UNAME_M),Darwinx86_64)
	TOOLCHAIN_PREFIX = aarch64-unknown-linux-gnu-
else ifeq ($(UNAME_S)$(UNAME_M),Linuxx86_64)
	TOOLCHAIN_PREFIX = aarch64-linux-gnu-
endif

CXX = $(TOOLCHAIN_PREFIX)c++
CXXFLAGS = -std=c++20\
           -Iinclude\
           -Iinclude/lib\
           -ffreestanding\
           -nostdinc\
           -nostdlib\
           -nostartfiles\
           -fno-threadsafe-statics\
           -fno-rtti\
           -fno-exceptions\
           -fno-stack-protector\
           -Wall

LD = $(TOOLCHAIN_PREFIX)ld
LDFLAGS = -T scripts/linker.ld

OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
OBJCOPYFLAGS = -O binary

GDB = $(TOOLCHAIN_PREFIX)gdb
GDBFLAGS = -x scripts/debug.gdb

BUILD_DIR = build
ELF = valkyrie.elf
IMG = kernel8.img
SRC = $(wildcard **/*.S) $(wildcard **/*.cc)
OBJ = boot.o $(filter-out boot.o, $(wildcard *.o))


all:
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	make valkyrie

valkyrie:
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(ELF) $(OBJ)
	$(OBJCOPY) $(OBJCOPYFLAGS) $(BUILD_DIR)/$(ELF) $(BUILD_DIR)/$(IMG)

run-debug:
	qemu-system-aarch64 -M raspi3b\
		-kernel $(BUILD_DIR)/$(IMG)\
		-drive if=sd,file=sd.img,format=raw\
		-display none\
		-serial null\
		-serial stdio\
		-S -s

run:
	qemu-system-aarch64 -M raspi3b\
		-kernel $(BUILD_DIR)/$(IMG)\
		-drive if=sd,file=sd.img,format=raw\
		-display none\
		-serial null\
		-serial stdio

gdb:
	$(GDB) $(GDBFLAGS)

clean:
	find . -type f -iname "*.o" | xargs rm
	rm -rf $(BUILD_DIR)
