// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/VirtualFileSystem.h>

#include <Algorithm.h>
#include <List.h>
#include <String.h>
#include <fs/File.h>
#include <fs/Stat.h>
#include <fs/Vnode.h>
#include <proc/Task.h>

namespace valkyrie::kernel {

VirtualFileSystem& VirtualFileSystem::get_instance() {
  static VirtualFileSystem instance;
  return instance;
}


VirtualFileSystem::VirtualFileSystem()
    : _rootfs(),
      _opened_files() {}


bool VirtualFileSystem::mount_rootfs(UniquePtr<FileSystem> fs) {
  _rootfs = { move(fs) };
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

  return parent->create_child(basename, content, size);
}

SharedPtr<File> VirtualFileSystem::open(const String& pathname, int options) {
  // 1. Lookup pathname from the root vnode.
  SharedPtr<Vnode> target = resolve_path(pathname);

  if (!target) {
    return nullptr;
  }

  auto it = _opened_files.find_if([&target](const auto& f) {
    return f->vnode.get() == target.get();
  });

  // If the file has already been opened
  if (it != _opened_files.end()) {
    return *it;
  }

  // Otherwise, we will open it now.
  _opened_files.push_back(make_shared<File>(*_rootfs.fs, target, options));
  _opened_files.back()->pos = 0;

  // 2. Create a new file descriptor for this vnode if found.

  // 3. Create a new file if O_CREAT is specified in flags.
  if (!(options & O_CREAT)) {
    return _opened_files.back();
  }

  target = create(pathname, nullptr, 0, 0, 0, 0);
  return _opened_files.back();
}

int VirtualFileSystem::close(SharedPtr<File> file) {
  // 1. release the file descriptor
  if (!file) {
    return -1;
  }

  auto it = _opened_files.find_if([file](const auto& f) {
    return f->vnode == file->vnode;
  });

  if (it == _opened_files.end()) {
    return -1;
  }
  
  if (it->use_count() == 1) {
    _opened_files.remove_if([file](const auto& f) {
      return f->vnode == file->vnode;
    });
  }

  file->pos = 0;
  return 0;
}

int VirtualFileSystem::write(SharedPtr<File> file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
  if (!file) {
    return -1;
  }

  auto new_content = make_unique<char[]>(len);
  memcpy(new_content.get(), buf, len);
  
  file->vnode->set_content(move(new_content));
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

  printk("file size = %d, file_pos = %d\n", file->vnode->get_size(), file->pos);
  size_t readable_size = file->vnode->get_size() - file->pos;
  len = min(len, readable_size);
  printk("vfs_read: memcpy(0x%x, 0x%x, %d)\n", buf, file->vnode->get_content() + file->pos, len);
  memcpy(buf, file->vnode->get_content() + file->pos, len);

  file->pos += len;
  return len;
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
