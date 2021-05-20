// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/TmpFS.h>

#include <fs/Stat.h>
#include <fs/VirtualFileSystem.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

TmpFS::TmpFS()
    : _next_inode_index(1),
      _root_vnode(make_shared<TmpFSInode>(*this, nullptr, "/", nullptr, 0, S_IFDIR, 0, 0)) {}


SharedPtr<Vnode> TmpFS::get_root_vnode() {
  return _root_vnode;
}



TmpFSInode::TmpFSInode(TmpFS& fs,
                       TmpFSInode* parent,
                       const String& name,
                       const char* content,
                       off_t size,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_inode_index++, size, mode, uid, gid),
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


SharedPtr<Vnode> TmpFSInode::create_child(const String& name,
                                          const char* content,
                                          off_t size,
                                          mode_t mode,
                                          uid_t uid,
                                          gid_t gid) {
  _children.push_back(make_shared<TmpFSInode>(_fs, this, name, content, size, mode, uid, gid));
  return _children.back();
}

void TmpFSInode::add_child(SharedPtr<Vnode> child) {
  _children.push_back(move(static_pointer_cast<TmpFSInode>(child)));
}

SharedPtr<Vnode> TmpFSInode::remove_child(const String& name) {
  SharedPtr<Vnode> removed_child;

  _children.remove_if([&removed_child, &name](auto& vnode) {
    return vnode->_name == name &&
           (removed_child = move(vnode), true);
  });

  if (!removed_child) [[unlikely]] {
    printk("tmpfs: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

SharedPtr<Vnode> TmpFSInode::get_child(const String& name) {
  auto it = _children.find_if([&name](auto& vnode) {
    return vnode->_name == name;
  });

  return (it != _children.end()) ? *it : nullptr;
}

SharedPtr<Vnode> TmpFSInode::get_ith_child(size_t i) {
  for (auto it = _children.begin(); it != _children.end(); it++) {
    if (it.index() == i) {
      return *it;
    }
  }
  return nullptr;
}

size_t TmpFSInode::get_children_count() const {
  return _children.size();
}


int TmpFSInode::chmod(const mode_t mode) {
  return -1;
}

int TmpFSInode::chown(const uid_t uid, const gid_t gid) {
  return -1;
}

}  // namespace valkyrie::kernel
