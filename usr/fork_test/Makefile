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

ELF = fork_test
SRC = ../vlibc/vlibc.cc main.cc ../vlibc/syscall.S ../vlibc/printf.cc
OBJ = vlibc.o main.o syscall.o printf.o

all:
	$(CXX) $(CXXFLAGS) -c $(SRC)
	$(LD) $(LDFLAGS) -o $(ELF) $(OBJ)

clean:
	rm $(ELF) $(OBJ)
