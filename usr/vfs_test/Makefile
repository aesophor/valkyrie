CXX = aarch64-linux-gnu-g++
CXXFLAGS = -std=c++20\
	   -I../vlibc\
	   -ffreestanding\
	   -nostdinc\
	   -nostdlib\
	   -nostartfiles\
	   -fno-threadsafe-statics\
	   -fno-rtti\
	   -fno-exceptions\
	   -fomit-frame-pointer\
	   -fno-asynchronous-unwind-tables\
	   -fno-unwind-tables\
	   -Wall

LD = aarch64-linux-gnu-ld
LDFLAGS = -T ../vlibc/linker.ld

ELF = vfs_test
SRC = ../vlibc/vlibc.cc ../vlibc/syscall.S ../vlibc/printf.cc main.cc
OBJ = vlibc.o syscall.o printf.o main.o

all:
	$(CXX) $(CXXFLAGS) -c $(SRC)
	$(LD) $(LDFLAGS) -o $(ELF) $(OBJ)

clean:
	rm $(ELF) $(OBJ)
