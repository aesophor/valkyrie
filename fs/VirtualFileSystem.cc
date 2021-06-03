// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/VirtualFileSystem.h>

#include <Algorithm.h>
#include <Hash.h>
#include <List.h>
#include <String.h>
#include <driver/SDCardDriver.h>
#include <fs/CPIOArchive.h>
#include <fs/DirectoryEntry.h>
#include <fs/File.h>
#include <fs/Stat.h>
#include <fs/TmpFS.h>
#include <fs/Vnode.h>
#include <kernel/Kernel.h>
#include <proc/Task.h>

namespace valkyrie::kernel {

uint32_t VFS::_next_dev_major = 1;

VFS& VFS::get_instance() {
  static VFS instance;
  return instance;
}


VFS::VFS()
    : _mounts(),
      _opened_files(),
      _storage_devices(),
      _registered_devices() {}


void VFS::mount_rootfs() {
  // TODO: currently it only supports SD card.
  auto sdcard = make_unique<StorageDevice>("sda", SDCardDriver::get_instance());
  _storage_devices.push_back(move(sdcard));

  mount_rootfs(_storage_devices.front()->get_first_partition().get_filesystem());
}

void VFS::mount_rootfs(SharedPtr<FileSystem> fs) {
  if (!_mounts.empty()) [[unlikely]] {
    Kernel::panic("VFS::mount_rootfs: root filesystem is already mounted!\n");
  }

  _mounts.push_back(make_unique<Mount>(fs, fs->get_root_vnode(), fs->get_root_vnode()));
}

void VFS::mount_rootfs(SharedPtr<FileSystem> fs, const CPIOArchive& archive) {
  mount_rootfs(fs);

  if (!archive.is_valid()) [[unlikely]] {
    printk("initramfs unpacking failed invalid magic at start of compressed archive\n");
    Kernel::panic("VFS::mount_rootfs: unable to mount rootfs\n");
  }

  archive.for_each([this](const auto& entry) {
    mode_t mode = (entry.content_len) ? S_IFREG : S_IFDIR;
    create(entry.pathname, entry.content, entry.content_len, mode, 0, 0);
  });
}

void VFS::mount_devtmpfs() {
  // Create `/dev` if it doesn't exist.
  static_cast<void>(mkdir("/dev"));

  // Mount devtmpfs or panic.
  if (mount("devtmpfs", "/dev", "tmpfs") == -1) [[unlikely]] {
    Kernel::panic("VFS::mount_devtmpfs: unable to mount devtmpfs\n");
  }
}

void VFS::populate_devtmpfs() {
  // Create `/dev/console` for a character device whose drvier is MiniUART.
  dev_t dev = register_device(Console::get_instance());
  mknod("/dev/console", S_IFCHR, Device::encode(Device::major(dev), 1));

  /*
  // Create `/dev/sdX` for each attached storage device.
  for (auto it = _storage_devices.begin(); it != _storage_devices.end(); it++) {
    static constexpr char table[5] = "abcd";
    char pathname[16] = {};
    sprintf(pathname, "/dev/sd%c", table[it.index()]);

    dev_t dev = register_device(**it);
    mknod(pathname, S_IFBLK, Device::encode(Device::major(dev), 1));
  }
  */
}


SharedPtr<Vnode> VFS::create(const String& pathname,
                             const char* content,
                             size_t size,
                             mode_t mode,
                             uid_t uid,
                             gid_t gid) {
  if (pathname == "." || pathname == "..") {
    return nullptr;
  }

  // Check if the parent directory entry exists.
  String basename;
  SharedPtr<Vnode> parent;
  SharedPtr<Vnode> target = resolve_path(pathname, &parent, &basename);

  // If the target already exists,
  // then we will return a `nullptr` to indicate
  // the create operation has failed.
  if (target) {
    return nullptr;
  }

  // If the parent doesn't even exist yet,
  // then we shouldn't create this dentry.
  if (!parent) {
    return nullptr;
  }

  return parent->create_child(basename, content, size, mode, 0, 0);
}

SharedPtr<File> VFS::open(const String& pathname, int options) {
  // Lookup pathname from the root vnode.
  SharedPtr<Vnode> target = resolve_path(pathname);

  // Check if this file has already been opened by any other process.
  auto it = _opened_files.find_if([&target](const auto& f) {
    return f->vnode.get() == target.get();
  });

  // If yes, then we can return its file handle now.
  if (it != _opened_files.end()) {
    return *it;
  }

  // Otherwise, the file has not been opened by any process yet.
  // If the file exists, then we will open it now.
  if (target) {
    _opened_files.push_back(make_shared<File>(get_rootfs(), target, options));
    return _opened_files.back();
  }

  // File doesn't exist...
  // We should check if `O_CREAT` is sepcified in options.
  if (!(options & O_CREAT)) {
    return nullptr;
  }

  // Okay, so the user wants to create this file...
  target = create(pathname, nullptr, 0, S_IFREG, 0, 0);
  _opened_files.push_back(make_shared<File>(get_rootfs(), target, options));
  return _opened_files.back();
}

int VFS::close(SharedPtr<File> file) {
  if (!file) [[unlikely]] {
    return -1;
  }

  auto it = _opened_files.find_if([file](const auto& f) {
    return f->vnode == file->vnode;
  });

  // `file` != nullptr but the file is not opened.
  // The user is probably trying to close an unopened file?
  if (it == _opened_files.end()) [[unlikely]] {
    return -1;
  }
  
  file->pos = 0;

  // Why should we release this file from the system-wide file table
  // when file.use_count == 3?
  //
  // 1. `file` != nullptr
  // 2. `_opened_files` contains one instance
  // 3. sys_close contains one instance
  if (file.use_count() <= 3) {
    _opened_files.remove_if([file](const auto& f) {
      return f->vnode == file->vnode;
    });
  }

  return 0;
}

int VFS::write(SharedPtr<File> file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  if (!file) [[unlikely]] {
    return -1;
  }

  if (file->vnode->is_regular_file()) {
    auto new_content = make_unique<char[]>(len);
    memcpy(new_content.get(), buf, len);
    file->vnode->set_content(move(new_content), len);

  } else if (file->vnode->is_character_device()) {
    auto cdev = static_cast<CharacterDevice*>(find_registered_device(file->vnode->get_dev()));

    if (cdev->get_name() == "console") {
      Console::get_instance().write(reinterpret_cast<const char*>(buf), len);
    } else {
      for (size_t i = 0; i < len; i++) {
        cdev->write_char(reinterpret_cast<const char*>(buf)[i]);
      }
    }

  } else {
    printk("VFS::write on this file type is not supported yet, mode = 0x%x\n", file->vnode->get_mode());
    return -1;
  }

  file->pos += len;
  return len;
}

int VFS::read(SharedPtr<File> file, void* buf, size_t len) {
  // 1. read min(len, readable file data size) byte to buf from the opened file.
  // 2. return read size or error code if an error occurs.
  if (!file) [[unlikely]] {
    return -1;
  }

  if (file->vnode->is_regular_file()) {
    size_t readable_size = file->vnode->get_size() - file->pos;
    len = min(len, readable_size);
    memcpy(buf, file->vnode->get_content() + file->pos, len);
    file->pos += len;

  } else if (file->vnode->is_directory()) {
    // Here `file->pos` is used as the index of the last iterated child.
    // FIXME: lol this is fooking slow, but i'm too busy this week...
    if (file->pos >= file->vnode->get_children_count()) {
      file->pos = 0;
      len = 0;
    } else {
      DirectoryEntry e;
      SharedPtr<Vnode> child_vnode = file->vnode->get_ith_child(file->pos);
      if (!child_vnode) [[unlikely]] {
        Kernel::panic("VFS::read: child_vnode == nullptr. "
                      "get_ith_child() is probably buggy.\n");
      }

      strncpy(e.name, child_vnode->get_name().c_str(), sizeof(e.name));
      memcpy(buf, reinterpret_cast<char*>(&e), sizeof(e));
      file->pos++;
      len = sizeof(e);
    }

  } else if (file->vnode->is_character_device()) {
    auto cdev = static_cast<CharacterDevice*>(find_registered_device(file->vnode->get_dev()));

    if (cdev->get_name() == "console") {
      Console::get_instance().read(reinterpret_cast<char*>(buf), len);
    } else {
      for (size_t i = 0; i < len; i++) {
        reinterpret_cast<char*>(buf)[i] = cdev->read_char();
      }
    }

  } else {
    printk("VFS::read: on this file type is not supported yet, mode = 0x%x\n", file->vnode->get_mode());
    return -1;
  }

  return len;
}

int VFS::access(const String& pathname, int options) {
  // Check user's permission for a file.
  // FIXME: currently it simply checks if the file exists...
  SharedPtr<Vnode> target = resolve_path(pathname);

  if (!target) {
    return -1;
  }

  return 0;
}

int VFS::mkdir(const String& pathname) {
  if (create(pathname, nullptr, 0, S_IFDIR, 0, 0)) {
    return 0;
  }
  return -1;
}

int VFS::rmdir(const String& pathname) {
  return -1;
}

int VFS::unlink(const String& pathname) {
  return -1;
}

int VFS::mount(const String& device_name,
               const String& mountpoint,
               const String& fs_name) {
  // Check if `mountpoint` exists.
  SharedPtr<Vnode> vnode = resolve_path(mountpoint);

  if (!vnode) [[unlikely]] {
    printk("VFS::mount: %s does not exist\n", mountpoint.c_str());
    return -1;
  }

  // For device-based filesystems, `device_name` should be
  // a pathname of a device file that stores a file system.
  // TODO: support device-based fs.

  // For memory-based filesystems, `device_name` can be used as
  // the name for the mounted file system.
  
  // `fs_name` is the filesystem’s name.
  // The VFS should find and call the filesystem’s method to set up the mount.
  if (fs_name == "tmpfs") {
    printk("VFS::mount: mounting tmpfs on %s\n", mountpoint.c_str());
    auto tmpfs = make_shared<TmpFS>();
    auto mount = make_unique<Mount>(tmpfs, tmpfs->get_root_vnode(), vnode);
    _mounts.push_back(move(mount));
  }

  return 0;
}

int VFS::umount(const String& mountpoint) {
  // Check if `mountpoint` is valid.
  SharedPtr<Vnode> vnode = resolve_path(mountpoint);

  if (!vnode) [[unlikely]] {
    printk("VFS::umount: %s does not exist\n", mountpoint.c_str());
    return -1;
  }

  auto it = _mounts.find_if([&vnode](const auto& mount) {
    return mount->guest_vnode == vnode;
  });

  if (it == _mounts.end()) [[unlikely]] {
    printk("VFS::umount: %s has not been mounted yet\n", mountpoint.c_str());
    return -1;
  }
  

  printk("VFS::umount: umounting %s\n", mountpoint.c_str());
  _mounts.remove_if([&vnode](const auto& mount) {
    return mount->guest_vnode == vnode;
  });
  // FIXME: free the corresponding fs
  return 0;
}

int VFS::mknod(const String& pathname, mode_t mode, dev_t dev) {
  // This special filesystem node must be either:
  // 1. a character device
  // 2. a block device
  // 3. a fifo device (pipe)
  if (!(mode & S_IFCHR) &&
      !(mode & S_IFBLK) &&
      !(mode & S_IFIFO)) [[unlikely]] {
    printk("VFS::mknod: invalid mode provided\n");
    return -1;
  }

  SharedPtr<Vnode> vnode = create(pathname, nullptr, 0, mode, 0, 0);

  if (!vnode) [[unlikely]] {
    printk("VFS::mknod: failed to create %s\n", pathname.c_str());
    return -1;
  }

  vnode->set_dev(dev);
  return 0;
}


SharedPtr<Vnode> VFS::get_mounted_vnode_or_host_vnode(SharedPtr<Vnode> vnode) {
  if (!vnode) {
    return nullptr;
  }

  auto it = _mounts.find_if([&vnode](const auto& mount) {
    return mount->host_vnode->hash_code() == vnode->hash_code();
  });

  return (it != _mounts.end()) ? (*it)->guest_vnode : vnode;
}


SharedPtr<Vnode> VFS::resolve_path(const String& pathname,
                                   SharedPtr<Vnode>* out_parent,
                                   String* out_basename) {
  if (pathname == ".") {
    // FIXME: what about out_parent and out_basename?
    return Task::current()->get_cwd_vnode();
  }

  List<String> components = pathname.split('/');

  if (components.empty()) {
    if (out_parent) {
      *out_parent = nullptr;
    }

    if (out_basename) {
      *out_basename = "";
    }

    return get_rootfs().get_root_vnode();
  }


  const bool absolute = pathname.front() == '/';
  auto root_vnode = (absolute) ? get_rootfs().get_root_vnode() :
                                 Task::current()->get_cwd_vnode();

  // If `pathname` is something like "/bin",
  // then we can simply check if "bin" exists under the root vnode.
  // Also, `out_parent` is simply the root_vnode.
  if (components.size() == 1) {
    if (out_parent) {
      *out_parent = root_vnode;
    }

    if (out_basename) {
      *out_basename = components.front();
    }

    return get_mounted_vnode_or_host_vnode(root_vnode->get_child(components.front()));
  }

  // Otherwise, we need to search the tree.
  auto vnode = root_vnode;

  for (auto it = components.begin(); it != components.end(); it++) {
    if (out_parent && it.index() == components.size() - 1) {
      *out_parent = vnode;
    }

    auto child = get_mounted_vnode_or_host_vnode(vnode->get_child(*it));

    if (!child) {
      vnode = nullptr;
      break;
    }
    vnode = child;
  }

  if (out_basename) {
    *out_basename = components.back();
  }

  return vnode;
}


dev_t VFS::register_device(Device& device) {
  dev_t dev = Device::encode(_next_dev_major++, 0);
  _registered_devices.push_back(Pair<dev_t, Device*>{dev, &device});
  return dev;
}

Device* VFS::find_registered_device(dev_t dev) {
  auto it = _registered_devices.find_if([dev](const auto& entry) {
    return Device::major(dev) == Device::major(entry.first);
  });

  return (it != _registered_devices.end()) ? it->second : nullptr;
}


FileSystem& VFS::get_rootfs() {
  return *(_mounts.front()->guest_fs);
}

List<SharedPtr<File>>& VFS::get_opened_files() {
  return _opened_files;
}


VFS::Mount::Mount(SharedPtr<FileSystem> guest_fs,
                  SharedPtr<Vnode> guest_vnode,
                  SharedPtr<Vnode> host_vnode)
    : guest_fs(move(guest_fs)),
      guest_vnode(move(guest_vnode)),
      host_vnode(move(host_vnode)) {}

}  // namespace valkyrie::kernel
