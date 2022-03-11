// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/TmpFS.h>

#include <CString.h>

#include <dev/Console.h>
#include <fs/Stat.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/Kernel.h>

namespace valkyrie::kernel {

TmpFS::TmpFS()
    : _root_inode(make_shared<TmpFSInode>(*this, nullptr, "/", nullptr, 0, S_IFDIR, 0, 0)) {}

SharedPtr<Vnode> TmpFS::get_root_vnode() {
  return _root_inode;
}

TmpFSInode::TmpFSInode(TmpFS &fs, SharedPtr<TmpFSInode> parent, const String &name,
                       const char *content, off_t size, mode_t mode, uid_t uid, gid_t gid)
    : Vnode(VFS::the().get_next_inode_idx(), size, mode, uid, gid),
      _fs(fs),
      _name(name),
      _content(),
      _parent(parent),
      _children() {
  if (content && size > 0) {
    _content = make_unique<char[]>(size);
    memcpy(_content.get(), content, size);
    _size = size;
  }
}

SharedPtr<Vnode> TmpFSInode::create_child(const String &name, const char *content, off_t size,
                                          mode_t mode, uid_t uid, gid_t gid) {
  auto inode =
      make_shared<TmpFSInode>(_fs, shared_from_this(), name, content, size, mode, uid, gid);
  _children.push_back(inode);
  return inode;
}

void TmpFSInode::add_child(SharedPtr<Vnode> child) {
  _children.push_back(move(static_pointer_cast<TmpFSInode>(child)));
}

SharedPtr<Vnode> TmpFSInode::remove_child(const String &name) {
  SharedPtr<Vnode> removed_child;

  _children.remove_if([&removed_child, &name](auto &vnode) {
    return vnode->_name == name && (removed_child = move(vnode), true);
  });

  if (!removed_child) [[unlikely]] {
    printk("TmpFS: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

SharedPtr<Vnode> TmpFSInode::get_child(const String &name) {
  if (name == ".") {
    return shared_from_this();
  } else if (name == "..") {
    return get_parent();
  }

  auto it = _children.find_if([&name](auto &vnode) { return vnode->_name == name; });

  return (it != _children.end()) ? *it : nullptr;
}

SharedPtr<Vnode> TmpFSInode::get_ith_child(size_t i) {
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

size_t TmpFSInode::get_children_count() const {
  return NR_SPECIAL_ENTRIES + _children.size();
}

SharedPtr<Vnode> TmpFSInode::get_parent() {
  if (is_root_vnode()) {
    return VFS::the().get_host_vnode(_fs._root_inode)->get_parent();
  }

  return _parent.lock();
}

void TmpFSInode::set_parent(SharedPtr<Vnode> parent) {
  _parent = parent;
}

int TmpFSInode::chmod(const mode_t mode) {
  return -1;
}

int TmpFSInode::chown(const uid_t uid, const gid_t gid) {
  return -1;
}

String TmpFSInode::get_name() const {
  return _name;
}

char *TmpFSInode::get_content() {
  return _content.get();
}

void TmpFSInode::set_content(UniquePtr<char[]> content, off_t new_size) {
  _content = move(content);
  _size = new_size;
}

size_t TmpFSInode::hash_code() const {
  return Hash<TmpFSInode>{}(*this);
}

bool TmpFSInode::is_root_vnode() const {
  return this == _fs._root_inode.get();
}

}  // namespace valkyrie::kernel
