// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
#define VALKYRIE_VIRTUAL_FILE_SYSTEM_H_

#include <List.h>
#include <Memory.h>
#include <fs/File.h>
#include <fs/FileSystem.h>

namespace valkyrie::kernel {

class VirtualFileSystem final {
 public:
  struct Mount final {
    UniquePtr<FileSystem> fs;
  };

  static VirtualFileSystem& get_instance();
  ~VirtualFileSystem() = default;

  bool mount_rootfs(UniquePtr<FileSystem> fs);

  File* open(const String& pathname, int flags);
  int close(File* file);
  int write(File* file, const void* buf, size_t len);
  int read(File* file, void* buf, size_t len);

  VirtualFileSystem::Mount& get_rootfs();
  List<SharedPtr<File>>& get_opened_files();

 private:
  VirtualFileSystem();

  Mount _rootfs;
  List<SharedPtr<File>> _opened_files;  // FIXME: replace it with a HashMap (?)
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
