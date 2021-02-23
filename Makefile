CXX = aarch64-linux-gnu-g++
CXXFLAGS = -Iinclude -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles

LD = aarch64-linux-gnu-ld
LDFLAGS = -T scripts/linker.ld

OBJCOPY = aarch64-linux-gnu-objcopy
OBJCOPYFLAGS = -O binary

GDB = aarch64-linux-gnu-gdb
GDBFLAGS = -x ./debug.gdb

BUILD_DIR = build
OS_NAME = valkyrie
SRC = $(wildcard **/*.S) $(wildcard **/*.cc)
OBJ = $(wildcard *.o)
OBJ := kloader.o $(filter-out kloader.o, $(OBJ))


all:
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	make valkyrie

valkyrie:
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/$(OS_NAME).elf $(OBJ)
	$(OBJCOPY) $(OBJCOPYFLAGS) $(BUILD_DIR)/$(OS_NAME).elf $(BUILD_DIR)/$(OS_NAME).img

run:
	qemu-system-aarch64 -M raspi3\
		-kernel $(BUILD_DIR)/$(OS_NAME).img\
		-display none\
		-S -s

debug:
	$(GDB) $(GDBFLAGS)

clean:
	find . -type f -iname "*.o" | xargs rm
	rm -rf $(BUILD_DIR)

