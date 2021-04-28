CXX = aarch64-linux-gnu-g++
CXXFLAGS = -std=c++20\
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
LDFLAGS = -T linker.ld

ELF = fork_test
SRC = main.cc printf.cc
OBJ = main.o printf.o

all:
	$(CXX) $(CXXFLAGS) -c $(SRC)
	$(LD) $(LDFLAGS) -o $(ELF) $(OBJ)

clean:
	rm $(ELF) $(OBJ)
