// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/TmpFS.h>

#include <fs/Stat.h>
#include <kernel/Compiler.h>

namespace valkyrie::kernel {

TmpFSInode::TmpFSInode(TmpFS& fs, TmpFSInode* parent, const String& name)
    : Inode(fs._next_inode_index++),
      _name(name),
      _parent(parent),
      _children() {}


void TmpFSInode::add_child(UniquePtr<Inode> child) {
  UniquePtr<TmpFSInode> c(static_cast<TmpFSInode*>(child.release()));
  _children.push_back(move(c));
}

UniquePtr<Inode> TmpFSInode::remove_child(const String& name) {
  UniquePtr<Inode> removed_child;

  _children.remove_if([&removed_child, &name](auto& inode) {
    return inode->_name == name &&
           (removed_child = move(inode), true);
  });

  if (unlikely(!removed_child)) {
    printk("tmpfs: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

int TmpFSInode::chmod(const mode_t mode) {

}

int TmpFSInode::chown(const uid_t uid, const gid_t gid) {

}



TmpFS::TmpFS()
    : _next_inode_index(1),
      _root_inode(make_unique<TmpFSInode>(*this, nullptr, "")) {}


void TmpFS::create(const String& pathname,
                   size_t size,
                   mode_t mode,
                   uid_t uid,
                   gid_t gid) {
  if (pathname == "." || pathname == "..") {
    return;
  }

  List<String> component_names = pathname.split('/');
  TmpFSInode* parent = _root_inode.get();

  for (const auto& name : component_names) {
    auto it = parent->_children.find_if([&name](const auto& inode) {
      return inode->_name == name;
    });

    // Create the child if not present.
    if (it != parent->_children.end()) {
      parent = it->get();
    } else {
      auto inode = make_unique<TmpFSInode>(*this, parent, name);
      parent->add_child(move(inode));
      parent = parent->_children.back().get();
    }
  }
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

Inode& TmpFS::get_root_inode() {
  return *_root_inode;
}

void TmpFS::debug_show_dfs_helper(TmpFSInode* inode, const int depth) const {
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
