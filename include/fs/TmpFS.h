// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TMP_FS_H_
#define VALKYRIE_TMP_FS_H_

#include <List.h>
#include <Memory.h>
#include <String.h>
#include <fs/File.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>

namespace valkyrie::kernel {

// Forward declaration
class TmpFS;


class TmpFSVnode final : public Vnode {
  // Friend declaration
  friend class TmpFS;

 public:
  TmpFSVnode(TmpFS& fs, TmpFSVnode* parent, const String& name);
  virtual ~TmpFSVnode() = default;

  virtual void add_child(UniquePtr<Vnode> child) override;
  virtual UniquePtr<Vnode> remove_child(const String& name) override;

  virtual int chmod(const mode_t mode) override;
  virtual int chown(const uid_t uid, const gid_t gid) override;

  const String& get_name() const { return _name; }

 private:
  String _name;

  TmpFSVnode* _parent;
  List<UniquePtr<TmpFSVnode>> _children;
};


class TmpFS final : public FileSystem {
  // Friend declaration
  friend class TmpFSVnode;
  friend class CPIOArchive;  // FIXME: wtf

 public:
  TmpFS();
  virtual ~TmpFS() = default;

  virtual Vnode* create(const String& pathname, size_t size, mode_t mode, uid_t uid, gid_t gid) override;
  virtual File* open(const String& pathname, int flags) override;
  virtual int close(File* file) override;
  virtual int write(File* file, const void* buf, size_t len) override;
  virtual int read(File* file, void* buf, size_t len) override;

  virtual void show() const override;
  virtual Vnode& get_root_vnode() override;
  virtual Vnode* get_vnode_by_pathname(const String& pathname) override;

 private:

  // FIXME: `inode` should be marked const, but it seems that
  // List<T>::ConstIterator isn't implemented properly...
  // I don't have the luxury (i.e. time) to fix this now, so maybe later...
  void debug_show_dfs_helper(TmpFSVnode* inode, const int depth) const;

  uint64_t _next_inode_index;
  UniquePtr<TmpFSVnode> _root_inode;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TMP_FS_H_
