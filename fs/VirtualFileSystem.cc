// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/VirtualFileSystem.h>

#include <Algorithm.h>
#include <List.h>
#include <String.h>
#include <driver/SDCardDriver.h>
#include <fs/CPIOArchive.h>
#include <fs/DirectoryEntry.h>
#include <fs/File.h>
#include <fs/Stat.h>
#include <fs/Vnode.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

VFS& VFS::get_instance() {
  static VFS instance;
  return instance;
}


VFS::VFS()
    : _rootfs(),
      _storage_devices(),
      _opened_files() {}


void VFS::initialize_attached_storage_devices() {
  // TODO: currently it only supports SD card.
  auto sdcard = make_unique<StorageDevice>(SDCardDriver::get_instance());
  _storage_devices.push_back(move(sdcard));

  mount_rootfs(_storage_devices.front()->get_first_partition().get_filesystem());
}

void VFS::mount_rootfs(FileSystem& fs) {
  _rootfs = { &fs };
}

/*
void VFS::mount_rootfs(FileSystem& fs, const CPIOArchive& archive) {
  _rootfs = { &fs };

  if (!archive.is_valid()) [[unlikely]] {
    printk("initramfs unpacking failed invalid magic at start of compressed archive\n");
    Kernel::panic("VFS: unable to mount root fs\n");
  }

  archive.for_each([this](const auto& entry) {
    mode_t mode = (entry.content_len) ? S_IFREG : S_IFDIR;
    create(entry.pathname, entry.content, entry.content_len, mode, 0, 0);
  });
}
*/


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
    _opened_files.push_back(make_shared<File>(*_rootfs.fs, target, options));
    return _opened_files.back();
  }

  // File doesn't exist...
  // We should check if `O_CREAT` is sepcified in options.
  if (!(options & O_CREAT)) {
    return nullptr;
  }

  // Okay, so the user wants to create this file...
  target = create(pathname, nullptr, 0, S_IFREG, 0, 0);
  _opened_files.push_back(make_shared<File>(*_rootfs.fs, target, options));
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
    size_t old_len = file->vnode->get_size();
    auto new_content = make_unique<char[]>(old_len + len);
    memcpy(new_content.get(), file->vnode->get_content(), old_len);
    memcpy(new_content.get() + old_len, buf, len);
    file->vnode->set_content(move(new_content), old_len + len);

  } else {
    printk("vfs_write on this file type is not supported yet, mode = 0x%x\n", file->vnode->get_mode());
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
      strncpy(e.name, child_vnode->get_name().c_str(), sizeof(e.name));
      memcpy(buf, reinterpret_cast<char*>(&e), sizeof(e));
      file->pos++;
      len = sizeof(e);
    }

  } else {
    printk("vfs_read on this file type is not supported yet, mode = 0x%x\n", file->vnode->get_mode());
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


SharedPtr<Vnode> VFS::resolve_path(const String& pathname,
                                   SharedPtr<Vnode>* out_parent,
                                   String* out_basename) const {
  List<String> components = pathname.split('/');

  if (components.empty()) {
    if (out_parent) {
      *out_parent = nullptr;
    }

    if (out_basename) {
      *out_basename = "";
    }

    return _rootfs.fs->get_root_vnode();
  }

  // If `pathname` is something like "/bin",
  // then we can simply check if "bin" exists under the root vnode.
  // Also, `out_parent` is simply the root_vnode.
  if (components.size() == 1) {
    auto root_vnode = _rootfs.fs->get_root_vnode();

    if (out_parent) {
      *out_parent = root_vnode;
    }

    if (out_basename) {
      *out_basename = components.front();
    }

    return root_vnode->get_child(components.front());
  }

  // Otherwise, we need to search the tree.
  auto vnode = _rootfs.fs->get_root_vnode();

  for (auto it = components.begin(); it != components.end(); it++) {
    if (out_parent && it.index() == components.size() - 1) {
      *out_parent = vnode;
    }

    auto child = vnode->get_child(*it);
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


FileSystem& VFS::get_rootfs() {
  return *_rootfs.fs;
}

List<SharedPtr<File>>& VFS::get_opened_files() {
  return _opened_files;
}

}  // namespace valkyrie::kernel
