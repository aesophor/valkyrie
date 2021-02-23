CXX = aarch64-linux-gnu-g++
CXXFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles
LD = aarch64-linux-gnu-ld
LDFLAGS = -T scripts/linker.ld
OBJCOPY = aarch64-linux-gnu-objcopy
OBJCOPYFLAGS = -O binary

BUILD = build
OS_NAME = valkyrie
SRC = $(wildcard **/*.S) $(wildcard **/*.cc)
OBJECTS = $(wildcard **/*.o) $(wildcard *.o)


all:
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	make valkyrie

valkyrie:
	$(LD) $(LDFLAGS) -o $(BUILD)/$(OS_NAME).elf kloader.o kmain.o
	$(OBJCOPY) $(OBJCOPYFLAGS) $(BUILD)/$(OS_NAME).elf $(BUILD)/$(OS_NAME).img

run:
	qemu-system-aarch64 -M raspi3\
		-kernel $(BUILD)/$(OS_NAME).img\
		-display none\
		-S -s

clean:
	find . -type f -iname "*.o" | xargs rm
	rm -rf $(BUILD)

