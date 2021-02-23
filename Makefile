CXX = aarch64-none-elf-gcc
LD = aarch64-none-elf-ld
LDFLAGS = -T scripts/linker.ld

BUILD = build
OS_NAME = valkyrie
SRC_CPP = $(wildcard **/*.cc) $(wildcard **/*.S)
OBJECTS = $(wildcard **/*.o) $(wildcard *.o)


all:
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $(SRC_CPP)
	make valkyrie

valkyrie:
	$(LD) $(LDFLAGS) -o $(BUILD)/$(OS_NAME).img $(OBJECTS)

run:
	qemu-system-aarch64 -M raspi3 -kernel $(BUILD)/$(OS_NAME).img -display none -d in_asm

clean:
	find . -type f -iname "*.o" | xargs rm
	rm -rf $(BUILD)
