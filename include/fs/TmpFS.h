// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TMP_FS_H_
#define VALKYRIE_TMP_FS_H_

#include <List.h>
#include <Memory.h>
#include <String.h>
#include <fs/File.h>
#include <fs/Inode.h>

namespace valkyrie::kernel {

// Forward declaration
class TmpFS;


class TmpFSInode final : public Inode {
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


class TmpFS final {
  friend class TmpFSInode;
  friend class CPIOArchive;  // FIXME: wtf

 public:
  TmpFS();
  virtual ~TmpFS() = default;

  virtual File* open(const String& pathname, int flags);
  virtual int close(File* file);
  virtual int write(File* file, const void* buf, size_t len);
  virtual int read(File* file, void* buf, size_t len);

 private:
  void create_dentry(const String& pathname, size_t size, mode_t mode, uid_t uid, gid_t gid);

  uint64_t _next_inode_index;
  UniquePtr<TmpFSInode> _root_inode;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TMP_FS_H_
