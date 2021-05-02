// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/TmpFS.h>

#include <fs/Stat.h>
#include <kernel/Compiler.h>

namespace valkyrie::kernel {

TmpFSVnode::TmpFSVnode(TmpFS& fs, TmpFSVnode* parent, const String& name)
    : Vnode(fs._next_inode_index++),
      _name(name),
      _parent(parent),
      _children() {}


void TmpFSVnode::add_child(UniquePtr<Vnode> child) {
  UniquePtr<TmpFSVnode> c(static_cast<TmpFSVnode*>(child.release()));
  _children.push_back(move(c));
}

UniquePtr<Vnode> TmpFSVnode::remove_child(const String& name) {
  UniquePtr<Vnode> removed_child;

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
    : _next_inode_index(1),
      _root_inode(make_unique<TmpFSVnode>(*this, nullptr, "")) {}


Vnode* TmpFS::create(const String& pathname,
                     size_t size,
                     mode_t mode,
                     uid_t uid,
                     gid_t gid) {
  if (pathname == "." || pathname == "..") {
    return nullptr;
  }

  List<String> component_names = pathname.split('/');
  TmpFSVnode* vnode = _root_inode.get();

  for (const auto& name : component_names) {
    auto it = vnode->_children.find_if([&name](const auto& v) {
      return v->_name == name;
    });

    // Create the child if not present.
    if (it != vnode->_children.end()) {
      vnode = it->get();
    } else {
      auto child = make_unique<TmpFSVnode>(*this, vnode, name);
      vnode->add_child(move(child));
      vnode = vnode->_children.back().get();
    }
  }

  return vnode;
}


File* TmpFS::open(const String& pathname, int flags) {

}

int TmpFS::close(File* file) {

}

int TmpFS::write(File* file, const void* buf, size_t len) {

}

int TmpFS::read(File* file, void* buf, size_t len) {

}


void TmpFS::show() const {
  printf("\n----- dumping rootfs tree -----\n");
  debug_show_dfs_helper(_root_inode.get(), -1);
  printf("----- end dumping rootfs tree -----\n");
}

Vnode& TmpFS::get_root_vnode() {
  return *_root_inode;
}

Vnode* TmpFS::get_vnode_by_pathname(const String& pathname) {
  List<String> component_names = pathname.split('/');
  TmpFSVnode* vnode = _root_inode.get();

  for (const auto& name : component_names) {
    auto it = vnode->_children.find_if([&name](const auto& v) {
      return v->_name == name;
    });

    if (it == vnode->_children.end()) {
      return nullptr;
    }
    vnode = it->get();
  }

  return vnode;
}

void TmpFS::debug_show_dfs_helper(TmpFSVnode* inode, const int depth) const {
  if (!inode) {
    return;
  }

  // Pre-order DFS
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }

  if (unlikely(inode != _root_inode.get())) {
    printf("%s\n", inode->_name.c_str());
  }

  for (const auto& child : inode->_children) {
    debug_show_dfs_helper(child.get(), depth + 1);
  }
}

}  // namespace valkyrie::kernel
