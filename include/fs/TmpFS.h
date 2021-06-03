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
class TmpFSInode;

class TmpFS final : public FileSystem {
  // Friend declaration
  friend class TmpFSInode;

 public:
  TmpFS();
  virtual ~TmpFS() = default;

  virtual SharedPtr<Vnode> get_root_vnode() override;

 private:
  uint64_t _next_inode_index;
  SharedPtr<TmpFSInode> _root_vnode;
};


class TmpFSInode final : public Vnode {
  // Friend declaration
  friend class TmpFS;
  friend struct Hash<TmpFSInode>;

 public:
  TmpFSInode(TmpFS& fs,
             TmpFSInode* parent,
             const String& name,
             const char* content,
             off_t size,
             mode_t mode,
             uid_t uid,
             gid_t gid);

  virtual ~TmpFSInode() = default;


  virtual SharedPtr<Vnode> create_child(const String& name,
                                        const char* content,
                                        off_t size,
                                        mode_t mode,
                                        uid_t uid,
                                        gid_t gid) override;
  virtual void add_child(SharedPtr<Vnode> child) override;
  virtual SharedPtr<Vnode> remove_child(const String& name) override;
  virtual SharedPtr<Vnode> get_child(const String& name) override;
  virtual SharedPtr<Vnode> get_ith_child(size_t i) override;
  virtual Vnode* get_parent() override;
  virtual size_t get_children_count() const override;

  virtual int chmod(const mode_t mode) override;
  virtual int chown(const uid_t uid, const gid_t gid) override;

  virtual String get_name() const override { return _name; }
  virtual char* get_content() override { return _content.get(); }
  virtual void set_content(UniquePtr<char[]> content, off_t new_size) override {
    _content = move(content);
    _size = new_size;
  }
  virtual size_t hash_code() const override;

 private:
  TmpFS& _fs;
  String _name;
  UniquePtr<char[]> _content;

  TmpFSInode* _parent;  // FIXME: use WeakPtr
  List<SharedPtr<TmpFSInode>> _children;
};


// Explicit (full) specialization of struct `Hash` for TmpFSInode.
template <>
struct Hash<TmpFSInode> {
  size_t operator ()(const TmpFSInode& inode) const {
    constexpr size_t prime = 11;
    size_t ret = 5;

    ret += prime * hash(inode._size);
    ret += prime * hash(inode._mode);
    ret += hash(inode._name);
    ret += prime * hash(inode._parent);
    return ret;
  }
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TMP_FS_H_
