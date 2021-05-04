// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/VirtualFileSystem.h>

#include <Algorithm.h>
#include <List.h>
#include <String.h>
#include <fs/CPIOArchive.h>
#include <fs/DirectoryEntry.h>
#include <fs/File.h>
#include <fs/Stat.h>
#include <fs/Vnode.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

VirtualFileSystem& VirtualFileSystem::get_instance() {
  static VirtualFileSystem instance;
  return instance;
}


VirtualFileSystem::VirtualFileSystem()
    : _rootfs(),
      _opened_files() {}


bool VirtualFileSystem::mount_rootfs(UniquePtr<FileSystem> fs,
                                     const CPIOArchive& archive) {
  _rootfs = { move(fs) };

  if (!archive.is_valid()) {
    printk("initramfs unpacking failed invalid magic at start of compressed archive\n");
    Kernel::panic("VFS: unable to mount root fs\n");
  }

  archive.for_each([this](const auto& entry) {
    mode_t mode = (entry.content_len) ? S_IFREG : S_IFDIR;
    create(entry.pathname, entry.content, entry.content_len, mode, 0, 0);
  });

  return _rootfs.fs;
}


SharedPtr<Vnode> VirtualFileSystem::create(const String& pathname,
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

SharedPtr<File> VirtualFileSystem::open(const String& pathname, int options) {
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

int VirtualFileSystem::close(SharedPtr<File> file) {
  if (!file) {
    return -1;
  }

  //printk("file handle use_count = %d\n", file.use_count());

  auto it = _opened_files.find_if([file](const auto& f) {
    return f->vnode == file->vnode;
  });

  if (it == _opened_files.end()) {
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

int VirtualFileSystem::write(SharedPtr<File> file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  if (!file) {
    return -1;
  }

  if (file->vnode->is_regular_file()) {
    auto new_content = make_unique<char[]>(len);
    memcpy(new_content.get(), buf, len);
    file->vnode->set_content(move(new_content));

  } else {
    printk("vfs_write on this file type is not supported yet, mode = 0x%x\n", file->vnode->get_mode());
    return -1;
  }

  file->pos += len;
  file->vnode->set_size(file->pos);
  return len;
}

int VirtualFileSystem::read(SharedPtr<File> file, void* buf, size_t len) {
  // 1. read min(len, readable file data size) byte to buf from the opened file.
  // 2. return read size or error code if an error occurs.
  if (!file) {
    return -1;
  }

  if (file->vnode->is_regular_file()) {
    size_t readable_size = file->vnode->get_size() - file->pos;
    len = min(len, readable_size);
    memcpy(buf, file->vnode->get_content() + file->pos, len);

  } else if (file->vnode->is_directory()) {

  } else {
    printk("vfs_read on this file type is not supported yet, mode = 0x%x\n", file->vnode->get_mode());
    return -1;
  }

  file->pos += len;
  return len;
}

int VirtualFileSystem::access(const String& pathname, int options) {
  // Check user's permission for a file.
  // FIXME: currently it simply checks if the file exists...
  SharedPtr<Vnode> target = resolve_path(pathname);

  if (!target) {
    return -1;
  }

  return 0;
}


SharedPtr<Vnode> VirtualFileSystem::resolve_path(const String& pathname,
                                                 SharedPtr<Vnode>* out_parent,
                                                 String* out_basename) const {
  List<String> components = pathname.split('/');

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


VirtualFileSystem::Mount& VirtualFileSystem::get_rootfs() {
  return _rootfs;
}

List<SharedPtr<File>>& VirtualFileSystem::get_opened_files() {
  return _opened_files;
}

}  // namespace valkyrie::kernel
