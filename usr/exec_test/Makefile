CXX = aarch64-linux-gnu-g++
CXXFLAGS = -std=c++20 -Iinclude -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-threadsafe-statics -fno-rtti -fno-exceptions -Wall

LD = aarch64-linux-gnu-ld
LDFLAGS = -T scripts/linker.ld

ELF = argv_test
SRC = argv_test.c
OBJ = argv_test.o

all:
	$(CXX) $(CXXFLAGS) -c $(SRC)
	$(LD) $(LDFLAGS) -o $(ELF) $(OBJ)
