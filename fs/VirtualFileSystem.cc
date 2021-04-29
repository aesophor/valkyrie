// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/VirtualFileSystem.h>

#include <Algorithm.h>
#include <fs/File.h>
#include <fs/Stat.h>
#include <fs/Vnode.h>

namespace valkyrie::kernel {

VirtualFileSystem& VirtualFileSystem::get_instance() {
  static VirtualFileSystem instance;
  return instance;
}


VirtualFileSystem::VirtualFileSystem()
    : _rootfs(),
      _opened_files() {}


bool VirtualFileSystem::mount_rootfs(UniquePtr<FileSystem> fs) {
  _rootfs = {move(fs)};
  // TODO: maybe add some error checking here...
  return _rootfs.fs;
}


File* VirtualFileSystem::open(const String& pathname, int flags) {
  // 1. Lookup pathname from the root vnode.
  // 2. Create a new file descriptor for this vnode if found.
  // 3. Create a new file if O_CREAT is specified in flags.
  FileSystem* rootfs = _rootfs.fs.get();

  if (unlikely(!rootfs)) {
    return nullptr;
  }

  Vnode* vnode = rootfs->get_vnode(pathname);

  if ((flags & O_CREAT) && !vnode) {
    vnode = rootfs->create(pathname, nullptr, 0, 0, 0, 0);
  }

  auto it = _opened_files.find_if([vnode](const auto& f) {
    return f->vnode == vnode;
  });

  File* ret = nullptr;

  if (it == _opened_files.end()) {
    auto file = make_unique<File>(*rootfs, vnode, flags);
    ret = file.get();
    _opened_files.push_back(move(file));
  } else {
    ret = it->get();
  }

  return ret;
}

int VirtualFileSystem::close(File* file) {
  // 1. release the file descriptor
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

  return 0;
}

int VirtualFileSystem::write(File* file, const void* buf, size_t len) {
  // 1. write len byte from buf to the opened file.
  // 2. return written size or error code if an error occurs.
}

int VirtualFileSystem::read(File* file, void* buf, size_t len) {
  // 1. read min(len, readable file data size) byte to buf from the opened file.
  // 2. return read size or error code if an error occurs.
  len = min(len, static_cast<decltype(len)>(file->vnode->get_size()));
  memcpy(buf, file->vnode->get_content(), len);
  return len;
}


VirtualFileSystem::Mount& VirtualFileSystem::get_rootfs() {
  return _rootfs;
}

List<SharedPtr<File>>& VirtualFileSystem::get_opened_files() {
  return _opened_files;
}

}  // namespace valkyrie::kernel
