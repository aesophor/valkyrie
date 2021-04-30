// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
#define VALKYRIE_VIRTUAL_FILE_SYSTEM_H_

#include <List.h>
#include <Memory.h>
#include <fs/CPIOArchive.h>
#include <fs/File.h>
#include <fs/FileSystem.h>

namespace valkyrie::kernel {

class VFS final {
 public:
  struct Mount final {
    UniquePtr<FileSystem> fs;
  };

  static VFS& get_instance();
  ~VFS() = default;

  bool mount_rootfs(UniquePtr<FileSystem> fs,
                    const CPIOArchive& archive);

  SharedPtr<Vnode> create(const String& pathname,
                          const char* content,
                          size_t size,
                          mode_t mode,
                          uid_t uid,
                          gid_t gid);

  SharedPtr<File> open(const String& pathname, int options);
  int close(SharedPtr<File> file);
  int write(SharedPtr<File> file, const void* buf, size_t len);
  int read(SharedPtr<File> file, void* buf, size_t len);
  int access(const String& pathname, int options);

  FileSystem& get_rootfs();
  List<SharedPtr<File>>& get_opened_files();

 private:
  VFS();

  // Retrieves the target vnode by `pathname`.
  SharedPtr<Vnode> resolve_path(const String& pathname,
                                SharedPtr<Vnode>* out_parent = nullptr,
                                String* out_basename = nullptr) const;
 


  Mount _rootfs;
  List<SharedPtr<File>> _opened_files;  // FIXME: replace it with a HashMap (?)
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
