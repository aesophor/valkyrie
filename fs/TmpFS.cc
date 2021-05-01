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
      _root_inode(make_unique<TmpFSInode>(*this, nullptr, "/")) {}


File* TmpFS::open(const String& pathname, int flags) {

}

int TmpFS::close(File* file) {

}

int TmpFS::write(File* file, const void* buf, size_t len) {

}

int TmpFS::read(File* file, void* buf, size_t len) {

}

}  // namespace valkyrie::kernel