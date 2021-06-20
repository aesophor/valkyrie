// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/ProcFS.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <fs/Stat.h>
#include <fs/VirtualFileSystem.h>
#include <libs/CString.h>

namespace {

bool is_switch_on = false;

}  // namespace

namespace valkyrie::kernel {

enum ProcFileType {
  PROC_SWITCH,
  PROC_HELLO,
  SIZE
};


ProcFS::ProcFS()
    : _next_inode_index(1),
      _root_inode(make_shared<ProcFSInode>(*this, nullptr, "/", S_IFDIR, ProcFS::FileType::DIR)) {
  // Lab 6: elective 3.1
  // The procfs creates switch and hello file in its root directory.
  // Users can access them by open, read, and write.
  _root_inode->create_child("switch", S_IFREG, ProcFS::FileType::SWITCH);
  _root_inode->create_child("hello", S_IFREG, ProcFS::FileType::HELLO);
}


SharedPtr<Vnode> ProcFS::get_root_vnode() {
  return _root_inode;
}



ProcFSInode::ProcFSInode(ProcFS& fs,
                         SharedPtr<ProcFSInode> parent,
                         const String& name,
                         mode_t mode,
                         ProcFS::FileType file_type)
    : Vnode(fs._next_inode_index++, 0, mode, 0, 0),
      _fs(fs),
      _name(name),
      _content(),
      _file_type(file_type),
      _parent(parent),
      _children() {}


SharedPtr<Vnode> ProcFSInode::create_child(const String& name,
                                           const char* content,
                                           off_t size,
                                           mode_t mode,
                                           uid_t uid,
                                           gid_t gid) {
  Kernel::panic("ProcFS: invalid version of `create_child()` called: %s\n", name.c_str());
}

SharedPtr<ProcFSInode> ProcFSInode::create_child(const String& name,
                                                 mode_t mode,
                                                 ProcFS::FileType file_type) {
  auto inode = make_shared<ProcFSInode>(_fs, shared_from_this(), name, mode, file_type);
  _children.push_back(inode);
  return inode;
}

void ProcFSInode::add_child(SharedPtr<Vnode> child) {
  _children.push_back(move(static_pointer_cast<ProcFSInode>(child)));
}

SharedPtr<Vnode> ProcFSInode::remove_child(const String& name) {
  SharedPtr<Vnode> removed_child;

  _children.remove_if([&removed_child, &name](auto& vnode) {
    return vnode->_name == name &&
           (removed_child = move(vnode), true);
  });

  if (!removed_child) [[unlikely]] {
    printk("ProcFS: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

SharedPtr<Vnode> ProcFSInode::get_child(const String& name) {
  if (name == ".") {
    return shared_from_this();
  } else if (name == "..") {
    return get_parent();
  }

  auto it = _children.find_if([&name](auto& vnode) {
    return vnode->_name == name;
  });

  return (it != _children.end()) ? *it : nullptr;
}

SharedPtr<Vnode> ProcFSInode::get_ith_child(size_t i) {
  if (i == 0) {
    return shared_from_this();
  } else if (i == 1) {
    return get_parent();
  }

  for (auto it = _children.begin(); it != _children.end(); it++) {
    if (NR_SPECIAL_ENTRIES + it.index() == i) {
      return *it;
    }
  }
  return nullptr;
}

size_t ProcFSInode::get_children_count() const {
  return NR_SPECIAL_ENTRIES + _children.size();
}

SharedPtr<Vnode> ProcFSInode::get_parent() {
  if (is_root_vnode()) {
    return VFS::get_instance().get_host_vnode(_fs._root_inode)->get_parent();
  }

  return _parent.lock();
}

void ProcFSInode::set_parent(SharedPtr<Vnode> parent) {
  _parent = parent;
}


int ProcFSInode::chmod(const mode_t mode) {
  return -1;
}

int ProcFSInode::chown(const uid_t uid, const gid_t gid) {
  return -1;
}

String ProcFSInode::get_name() const {
  return _name;
}

char* ProcFSInode::get_content() {
  using FileType = ProcFS::FileType;
  _content.reset();

  switch (_file_type) {
    case FileType::SWITCH:
      _content = move(get_content_for_switch());
      break;

    case FileType::HELLO:
      _content = move(get_content_for_hello());
      break;

    default:
      break;
  }

  return _content.get();
}

void ProcFSInode::set_content(UniquePtr<char[]> content, off_t) {
  using FileType = ProcFS::FileType;

  switch (_file_type) {
    case FileType::SWITCH:
      set_content_for_switch(move(content));
      break;

    case FileType::HELLO:
      set_content_for_hello(move(content));
      break;

    default:
      break;
  }
}

size_t ProcFSInode::hash_code() const {
  return Hash<ProcFSInode>{}(*this);
}

bool ProcFSInode::is_root_vnode() const {
  return this == _fs._root_inode.get();
}


UniquePtr<char[]> ProcFSInode::get_content_for_switch() {
  return nullptr;
}

UniquePtr<char[]> ProcFSInode::get_content_for_hello() {
  constexpr size_t len = 5;
  auto buffer = make_unique<char[]>(len);

  if (::is_switch_on) {
    strncpy(buffer.get(), "HELLO", len);
  } else {
    strncpy(buffer.get(), "hello", len);
  }

  _size = len;
  return buffer;
}

void ProcFSInode::set_content_for_switch(UniquePtr<char[]> content) {
  ::is_switch_on = static_cast<bool>(atoi(content.get()));
}

void ProcFSInode::set_content_for_hello(UniquePtr<char[]> content) {

}

}  // namespace valkyrie::kernel
