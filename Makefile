# Copyright (c) 2021-2022 Marco Wang <m.aesophor@gmail.com>. All rights reserved.

TOOLCHAIN_PREFIX_MACOS = aarch64-unknown-linux-gnu-
TOOLCHAIN_PREFIX_LINUX = aarch64-linux-gnu-
TOOLCHAIN_PREFIX = $(TOOLCHAIN_PREFIX_MACOS)

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
