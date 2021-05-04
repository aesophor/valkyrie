// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/TmpFS.h>

#include <fs/Stat.h>
#include <fs/VirtualFileSystem.h>
#include <kernel/Compiler.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

TmpFSVnode::TmpFSVnode(TmpFS& fs,
                       TmpFSVnode* parent,
                       const String& name,
                       const char* content,
                       size_t size,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_vnode_index++, mode, uid, gid),
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

TmpFSVnode::~TmpFSVnode() {
  printk("tmpfs: destructing vnode: %s\n", _name.c_str());
}


SharedPtr<Vnode> TmpFSVnode::create_child(const String& name,
                                          const char* content,
                                          size_t size,
                                          mode_t mode,
                                          uid_t uid,
                                          gid_t gid) {
  _children.push_back(make_shared<TmpFSVnode>(_fs, this, name, content, size, mode, uid, gid));
  return _children.back();
}

void TmpFSVnode::add_child(SharedPtr<Vnode> child) {
  _children.push_back(move(static_pointer_cast<TmpFSVnode>(child)));
}

SharedPtr<Vnode> TmpFSVnode::remove_child(const String& name) {
  SharedPtr<Vnode> removed_child;

  _children.remove_if([&removed_child, &name](auto& vnode) {
    return vnode->_name == name &&
           (removed_child = move(vnode), true);
  });

  if (unlikely(!removed_child)) {
    printk("tmpfs: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

SharedPtr<Vnode> TmpFSVnode::get_child(const String& name) {
  auto it = _children.find_if([&name](auto& vnode) {
    return vnode->_name == name;
  });

  return (it != _children.end()) ? *it : nullptr;
}

int TmpFSVnode::chmod(const mode_t mode) {
  return -1;
}

int TmpFSVnode::chown(const uid_t uid, const gid_t gid) {
  return -1;
}



TmpFS::TmpFS()
    : _next_vnode_index(1),
      _root_vnode(make_shared<TmpFSVnode>(*this, nullptr, "/", nullptr, 0, S_IFDIR, 0, 0)) {}



void TmpFS::show() const {
  printf("\n----- dumping rootfs tree -----\n");
  debug_show_dfs_helper(_root_vnode.get(), -1);
  printf("----- end dumping rootfs tree -----\n");
}

SharedPtr<Vnode> TmpFS::get_root_vnode() {
  return _root_vnode;
}

void TmpFS::debug_show_dfs_helper(TmpFSVnode* vnode, const int depth) const {
  if (!vnode) {
    return;
  }

  // Pre-order DFS
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }

  if (unlikely(vnode != _root_vnode.get())) {
    printf("%s\n", vnode->_name.c_str());
  }

  for (const auto& child : vnode->_children) {
    debug_show_dfs_helper(child.get(), depth + 1);
  }
}

}  // namespace valkyrie::kernel
