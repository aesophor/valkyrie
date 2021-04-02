## Install Cross-Compilation Toolchains

Arch Linux
```
$ sudo pacman -S aarch64-linux-gnu-gcc \
                 aarch64-linux-gnu-gdb \
                 qemu-arch-extra
```

## Build

```
$ git clone https://github.com/aesophor/valkyrie.git
$ cd valkyrie
$ make
```

The kernel image file `kernel8.img` can be located at `build/kernel8.img`.

## Run (Normal)

This method allows you to interact with the kernel via stdin/stdout
```
$ make run
```

## Run (Debugging with GDB)

This method should only be used if you want to debug this kernel with GDB.
```
$ make run-debug
```

Now, spawn another shell and run the following command, and the gdb will attach to ther kernel we've just run.
```
$ make gdb
```
