// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_STAT_H_
#define VALKYRIE_STAT_H_

// open() options
#define O_RDONLY    (1 <<  0)
#define O_WRONLY    (1 <<  1)
#define O_RDWR      (O_RDONLY | O_WRONLY)
#define O_ACCMODE   (O_RDONLY | O_WRONLY)
#define O_EXEC      (1 <<  2)
#define O_CREAT     (1 <<  3)
#define O_EXCL      (1 <<  4)
#define O_NOCTTY    (1 <<  5)
#define O_TRUNC     (1 <<  6)
#define O_APPEND    (1 <<  7)
#define O_NONBLOCK  (1 <<  8)
#define O_DIRECTORY (1 <<  9)
#define O_NOFOLLOW  (1 << 10)
#define O_CLOEXEC   (1 << 11)
#define O_DIRECT    (1 << 12)

// access() options
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0

// Directory entry permission flags
#define S_IRWXU 0000700  // RWX mask for owner
#define S_IRUSR 0000400  // R for owner
#define S_IWUSR 0000200  // W for owner
#define S_IXUSR 0000100  // X for owner

#define S_IRWXG 0000070  // RWX mask for group
#define S_IRGRP 0000040  // R for group
#define S_IWGRP 0000020  // W for group
#define S_IXGRP 0000010  // X for group

#define S_IRWXO 0000007  // RWX mask for other
#define S_IROTH 0000004  // R for other
#define S_IWOTH 0000002  // W for other
#define S_IXOTH 0000001  // X for other

#define S_ISUID 0004000  // set user id on execution
#define S_ISGID 0002000  // set group id on execution
#define S_ISVTX 0001000  // save swapped text even after use


// Directory entry type flags
#define S_IFMT   0170000  // file type bitmask
#define S_IFDIR  0040000  // file type is directory
#define S_IFCHR  0020000  // file type is character device
#define S_IFBLK  0060000  // file type is block device
#define S_IFREG  0100000  // file type is regular file
#define S_IFIFO  0010000  // file type is fifo
#define S_IFLNK  0120000  // file type is symlink
#define S_IFSOCK 0140000  // file type is socket

#endif  // VALKYRIE_STAT_H_
