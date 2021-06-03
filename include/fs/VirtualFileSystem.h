// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
#define VALKYRIE_VIRTUAL_FILE_SYSTEM_H_

#include <List.h>
#include <Memory.h>
#include <Utility.h>
#include <dev/Device.h>
#include <dev/StorageDevice.h>
#include <fs/CPIOArchive.h>
#include <fs/File.h>
#include <fs/FileSystem.h>

namespace valkyrie::kernel {

class VFS final {
 public:
  struct Mount final {
    Mount(SharedPtr<FileSystem> guest_fs,
          SharedPtr<Vnode> guest_vnode,
          SharedPtr<Vnode> host_vnode);

    SharedPtr<FileSystem> guest_fs;
    SharedPtr<Vnode> guest_vnode;
    SharedPtr<Vnode> host_vnode;
  };

  static VFS& get_instance();

  ~VFS() = default;
  VFS(const VFS&) = delete;
  VFS(VFS&&) = delete;
  VFS& operator =(const VFS&) = delete;
  VFS& operator =(VFS&&) = delete;


  void mount_rootfs();
  void mount_devtmpfs();
  void populate_devtmpfs();


  SharedPtr<File> open(const String& pathname, int options);
  int close(SharedPtr<File> file);
  int write(SharedPtr<File> file, const void* buf, size_t len);
  int read(SharedPtr<File> file, void* buf, size_t len);
  int access(const String& pathname, int options);
  int mkdir(const String& pathname);
  int rmdir(const String& pathname);
  int unlink(const String& pathname);
  int mount(const String& device_name, const String& mountpoint, const String& fs_name);
  int umount(const String& mountpoint);
  int mknod(const String& pathname, mode_t mode, dev_t dev);

  // Retrieves the target vnode by `pathname`.
  [[nodiscard]]
  SharedPtr<Vnode> resolve_path(const String& pathname,
                                SharedPtr<Vnode>* out_parent = nullptr,
                                String* out_basename = nullptr);

  // The API for each device to register itself to the VFS.
  [[nodiscard]] dev_t register_device(Device& device);


  [[nodiscard]] FileSystem& get_rootfs();
  [[nodiscard]] List<SharedPtr<File>>& get_opened_files();

 private:
  VFS();

  void mount_rootfs(SharedPtr<FileSystem> fs);
  void mount_rootfs(SharedPtr<FileSystem> fs, const CPIOArchive& archive);

  SharedPtr<Vnode> create(const String& pathname,
                          const char* content,
                          size_t size,
                          mode_t mode,
                          uid_t uid,
                          gid_t gid);

  [[nodiscard]] Device* find_registered_device(dev_t dev);

  [[nodiscard]]
  SharedPtr<Vnode> get_mounted_vnode_or_host_vnode(SharedPtr<Vnode> vnode);


  static uint32_t _next_dev_major;

  List<UniquePtr<Mount>> _mounts;
  List<SharedPtr<File>> _opened_files;  // FIXME: replace it with a HashMap (?)
  List<UniquePtr<StorageDevice>> _storage_devices;
  List<Pair<dev_t, Device*>> _registered_devices;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VIRTUAL_FILE_SYSTEM_H_
