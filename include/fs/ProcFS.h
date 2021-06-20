// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PROC_FS_H_
#define VALKYRIE_PROC_FS_H_

#include <List.h>
#include <Memory.h>
#include <String.h>
#include <fs/File.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>
#include <proc/Task.h>

namespace valkyrie::kernel {

// Forward declaration
class ProcFSInode;


class ProcFS final : public FileSystem {
  // Friend declaration
  friend class ProcFSInode;

 public:
  ProcFS();
  virtual ~ProcFS() = default;

  virtual SharedPtr<Vnode> get_root_vnode() override;

  void create_subtree_for_task(Task* task);

 private:
  uint64_t _next_inode_index;
  SharedPtr<ProcFSInode> _root_inode;
};


class ProcFSInode : public Vnode, public EnableSharedFromThis<ProcFSInode> {
  // Friend declaration
  friend class ProcFS;
  friend struct Hash<ProcFSInode>;

 public:
  enum class Type {
    DIR,
    SWITCH,
    HELLO,
    TASK_DIR,
    TASK_FILE_STATE,
    SIZE
  };

  ProcFSInode(ProcFS& fs,
              SharedPtr<ProcFSInode> parent,
              const String& name,
              mode_t mode,
              ProcFSInode::Type type);

  virtual ~ProcFSInode() = default;


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
  virtual size_t get_children_count() const override;
  virtual SharedPtr<Vnode> get_parent() override;
  virtual void set_parent(SharedPtr<Vnode> parent) override;

  virtual int chmod(const mode_t mode) override;
  virtual int chown(const uid_t uid, const gid_t gid) override;

  virtual String get_name() const override;
  virtual char* get_content() override;
  virtual void set_content(UniquePtr<char[]> content, off_t new_size) override;
  virtual size_t hash_code() const override;
  virtual bool is_root_vnode() const override;

  // Overloaded version of create_child() specialized for ProcFS.
  SharedPtr<ProcFSInode> create_child(const String& name,
                                      mode_t mode,
                                      ProcFSInode::Type type);
 private:
  UniquePtr<char[]> get_content_for_switch();
  UniquePtr<char[]> get_content_for_hello();

  void set_content_for_switch(UniquePtr<char[]> content);
  void set_content_for_hello(UniquePtr<char[]> content);


  ProcFS& _fs;
  String _name;
  UniquePtr<char[]> _content;
  ProcFSInode::Type _type;

  WeakPtr<Vnode> _parent;
  List<SharedPtr<ProcFSInode>> _children;
};


// There are multiple types of directories and files in ProcFS.
// We will use polymorphism to specialize the open/read/write method
// for different inodes in ProcFS, . Here we go...
class SwitchFile : public ProcFSInode {
 public:
};


// Explicit (full) specialization of struct `Hash` for ProcFSInode.
template <>
struct Hash<ProcFSInode> {
  size_t operator ()(const ProcFSInode& inode) const {
    constexpr size_t prime = 11;
    size_t ret = 5;

    ret += prime * hash(inode._mode);
    ret += prime * hash(static_cast<int>(inode._type));
    ret += hash(inode._name);
    ret += prime * hash(inode._parent);
    return ret;
  }
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PROC_FS_H_
