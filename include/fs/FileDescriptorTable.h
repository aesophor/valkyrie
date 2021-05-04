// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_FILE_DESCRIPTOR_TABLE_H_
#define VALKYRIE_FILE_DESCRIPTOR_TABLE_H_

#include <SharedPtr.h>
#include <fs/File.h>

#define NR_PROCESS_FD_LIMITS 16

namespace valkyrie::kernel {

class FileDescriptorTable final {
 public:
  FileDescriptorTable();

  int allocate_one_file_descriptor(SharedPtr<File> file);
  void deallocate_file_descriptor(const int fd);

 private:
  SharedPtr<File> _fd_array[NR_PROCESS_FD_LIMITS];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_STAT_H_
