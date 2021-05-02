// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TMP_FS_H_
#define VALKYRIE_TMP_FS_H_

#include <List.h>
#include <Memory.h>
#include <String.h>
#include <fs/File.h>
#include <fs/FileSystem.h>
#include <fs/Inode.h>

namespace valkyrie::kernel {

// Forward declaration
class TmpFS;


class TmpFSInode final : public Inode {
  // Friend declaration
  friend class TmpFS;

 public:
  TmpFSInode(TmpFS& fs, TmpFSInode* parent, const String& name);
  virtual ~TmpFSInode() = default;

  virtual void add_child(UniquePtr<Inode> child) override;
  virtual UniquePtr<Inode> remove_child(const String& name) override;

  virtual int chmod(const mode_t mode) override;
  virtual int chown(const uid_t uid, const gid_t gid) override;

  const String& get_name() const { return _name; }

 private:
  String _name;

  TmpFSInode* _parent;
  List<UniquePtr<TmpFSInode>> _children;
};


class TmpFS final : public FileSystem {
  // Friend declaration
  friend class TmpFSInode;
  friend class CPIOArchive;  // FIXME: wtf

 public:
  TmpFS();
  virtual ~TmpFS() = default;

  virtual void create(const String& pathname, size_t size, mode_t mode, uid_t uid, gid_t gid) override;
  virtual File* open(const String& pathname, int flags) override;
  virtual int close(File* file) override;
  virtual int write(File* file, const void* buf, size_t len) override;
  virtual int read(File* file, void* buf, size_t len) override;

  virtual void show() const override;
  virtual Inode& get_root_inode() override;

 private:

  // FIXME: `inode` should be marked const, but it seems that
  // List<T>::ConstIterator isn't implemented properly...
  // I don't have the luxury (i.e. time) to fix this now, so maybe later...
  void debug_show_dfs_helper(TmpFSInode* inode, const int depth) const;

  uint64_t _next_inode_index;
  UniquePtr<TmpFSInode> _root_inode;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TMP_FS_H_
