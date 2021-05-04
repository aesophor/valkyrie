// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FileDescriptorTable.h>

#include <proc/Task.h>

namespace valkyrie::kernel {

FileDescriptorTable::FileDescriptorTable() : _fd_array() {
  // Reserve fd = 0,1,2 for stdin, stdout, stderr
  _fd_array[0] = File::opened;
  _fd_array[1] = File::opened;
  _fd_array[2] = File::opened;
}


int FileDescriptorTable::allocate_one_file_descriptor(SharedPtr<File> file) {
  for (int i = 0; i < NR_PROCESS_FD_LIMITS; i++) {
    if (!_fd_array[i]) {
      _fd_array[i] = file;
      return i;
    }
  }

  printk("warning: task (pid = %d) fd table is full\n", Task::get_current().get_pid());
  return -1;
}

void FileDescriptorTable::deallocate_file_descriptor(const int fd) {
  _fd_array[fd] = nullptr;
}

}  // namespace valkyrie::kernel
