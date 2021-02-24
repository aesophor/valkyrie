CXX = aarch64-linux-gnu-g++
CXXFLAGS = -Iinclude -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles

LD = aarch64-linux-gnu-ld
LDFLAGS = -T scripts/linker.ld

OBJCOPY = aarch64-linux-gnu-objcopy
OBJCOPYFLAGS = -O binary

GDB = aarch64-linux-gnu-gdb
GDBFLAGS = -x ./debug.gdb

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
	qemu-system-aarch64 -M raspi3\
		-kernel $(BUILD_DIR)/$(IMG)\
		-display none\
		-serial null\
		-serial stdio\
		-S -s

run:
	qemu-system-aarch64 -M raspi3\
		-kernel $(BUILD_DIR)/$(IMG)\
		-display none\
		-serial null\
		-serial stdio

gdb:
	$(GDB) $(GDBFLAGS)

clean:
	find . -type f -iname "*.o" | xargs rm
	rm -rf $(BUILD_DIR)

