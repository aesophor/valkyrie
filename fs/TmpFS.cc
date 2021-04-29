// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/TmpFS.h>

#include <fs/Stat.h>
#include <kernel/Compiler.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

TmpFSVnode::TmpFSVnode(TmpFS& fs,
                       TmpFSVnode* parent,
                       const String& name,
                       const char* content,
                       size_t len)
    : Vnode(fs._next_vnode_index++),
      _name(name),
      _content(),
      _parent(parent),
      _children() {
  if (content && len > 0) {
    _content = make_unique<char[]>(len);
    memcpy(_content.get(), content, len);
  }
}

TmpFSVnode::~TmpFSVnode() {
  printk("tmpfs: destructing vnode: %s\n", _name.c_str());
}


void TmpFSVnode::add_child(SharedPtr<Vnode> child) {
  _children.push_back(move(static_pointer_cast<TmpFSVnode>(child)));
}

SharedPtr<Vnode> TmpFSVnode::remove_child(const String& name) {
  SharedPtr<Vnode> removed_child;

  _children.remove_if([&removed_child, &name](auto& inode) {
    return inode->_name == name &&
           (removed_child = move(inode), true);
  });

  if (unlikely(!removed_child)) {
    printk("tmpfs: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

int TmpFSVnode::chmod(const mode_t mode) {

}

int TmpFSVnode::chown(const uid_t uid, const gid_t gid) {

}



TmpFS::TmpFS()
    : _next_vnode_index(1),
      _root_vnode(make_shared<TmpFSVnode>(*this, nullptr, "", nullptr, 0)) {}


SharedPtr<Vnode> TmpFS::create(const String& pathname,
                               const char* content,
                               size_t size,
                               mode_t mode,
                               uid_t uid,
                               gid_t gid) {
  if (pathname == "." || pathname == "..") {
    return nullptr;
  }

  List<String> component_names = pathname.split('/');
  TmpFSVnode* vnode = _root_vnode.get();

  for (const auto& name : component_names) {
    auto it = vnode->_children.find_if([&name](const auto& v) {
      return v->_name == name;
    });

    // Create the child if not present.
    if (it != vnode->_children.end()) {
      vnode = it->get();
    } else {
      auto child = make_shared<TmpFSVnode>(*this, vnode, name, content, size);
      vnode->add_child(move(child));
      vnode = vnode->_children.back().get();
    }
  }

  return vnode;
}


SharedPtr<File> TmpFS::open(const String& pathname, int flags) {
  // 1. Lookup `pathname` from the root vnode.
  // 2. Create a new file descriptor for this vnode if doun.
  // 3. Create a new file if O_CREAT is specified in `flags`.
  SharedPtr<Vnode> vnode = get_vnode(pathname);

  if (!vnode && (flags & O_CREAT)) {
    vnode = create(pathname, nullptr, 0, 0, 0, 0);
  }

}

int TmpFS::close(SharedPtr<File> file) {

}

int TmpFS::write(SharedPtr<File> file, const void* buf, size_t len) {

}

int TmpFS::read(SharedPtr<File> file, void* buf, size_t len) {

}


void TmpFS::show() const {
  printf("\n----- dumping rootfs tree -----\n");
  debug_show_dfs_helper(_root_vnode.get(), -1);
  printf("----- end dumping rootfs tree -----\n");
}

SharedPtr<Vnode> TmpFS::get_root_vnode() {
  return _root_vnode;
}

SharedPtr<Vnode> TmpFS::get_vnode(const String& pathname) {
  List<String> component_names = pathname.split('/');
  SharedPtr<TmpFSVnode> vnode = _root_vnode;

  for (const auto& name : component_names) {
    auto it = vnode->_children.find_if([&name](const auto& v) {
      return v->_name == name;
    });

    if (it == vnode->_children.end()) {
      return nullptr;
    }
    vnode = *it;
  }

  return vnode;
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
