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

ELF = unlink
SRC = ../vlibc/vlibc.cc ../vlibc/syscall.S ../vlibc/printf.cc ../vlibc/cstring.cc main.cc
OBJ = vlibc.o main.o syscall.o printf.o cstring.o

all:
	$(CXX) $(CXXFLAGS) -o $(ELF) $(SRC)

clean:
	rm $(ELF) $(OBJ)
