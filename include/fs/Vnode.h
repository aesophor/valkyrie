// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VNODE_H_
#define VALKYRIE_VNODE_H_

#include <Hash.h>
#include <Memory.h>
#include <String.h>
#include <Types.h>

#include <dev/Device.h>
#include <fs/Stat.h>

namespace valkyrie::kernel {

class Vnode {
 public:
  Vnode(const uint32_t index, off_t size, mode_t mode, uid_t uid, gid_t gid)
      : _index(index), _size(size), _mode(mode), _uid(uid), _gid(gid), _dev() {}

  virtual ~Vnode() = default;

  virtual SharedPtr<Vnode> create_child(const String &name, const char *content, off_t size,
                                        mode_t mode, uid_t uid, gid_t gid) = 0;
  virtual void add_child(SharedPtr<Vnode> child) = 0;
  virtual SharedPtr<Vnode> remove_child(const String &name) = 0;
  virtual SharedPtr<Vnode> get_child(const String &name) = 0;
  virtual SharedPtr<Vnode> get_ith_child(size_t i) = 0;
  virtual size_t get_children_count() const = 0;
  virtual SharedPtr<Vnode> get_parent() = 0;
  virtual void set_parent(SharedPtr<Vnode> parent) = 0;

  virtual int chmod(const mode_t mode) = 0;
  virtual int chown(const uid_t uid, const gid_t gid) = 0;

  virtual String get_name() const = 0;
  virtual char *get_content() = 0;
  virtual void set_content(UniquePtr<char[]> content, off_t new_size) = 0;
  virtual size_t hash_code() const = 0;
  virtual bool is_root_vnode() const = 0;

  bool is_directory() const {
    return Vnode::is_directory(_mode);
  }

  bool is_regular_file() const {
    return Vnode::is_regular_file(_mode);
  }

  bool is_character_device() const {
    return Vnode::is_character_device(_mode);
  }

  bool is_block_device() const {
    return Vnode::is_block_device(_mode);
  }

  bool is_fifo() const {
    return Vnode::is_fifo(_mode);
  }

  bool is_symlink() const {
    return Vnode::is_symlink(_mode);
  }

  bool is_socket() const {
    return Vnode::is_socket(_mode);
  }

  bool is_sticky() const {
    return _mode & S_ISVTX;
  }

  bool is_setuid() const {
    return _mode & S_ISUID;
  }

  bool is_setgid() const {
    return _mode & S_ISGID;
  }

  uint32_t get_index() const {
    return _index;
  }

  off_t get_size() const {
    return _size;
  }

  mode_t get_mode() const {
    return _mode;
  }

  time_t get_ctime() const {
    return _ctime;
  }

  time_t get_atime() const {
    return _atime;
  }

  time_t get_mtime() const {
    return _mtime;
  }

  dev_t get_dev() const {
    return _dev;
  }

  uint32_t get_dev_major() const {
    return Device::major(_dev);
  }

  uint32_t get_dev_minor() const {
    return Device::minor(_dev);
  }

  void set_size(off_t size) {
    _size = size;
  }

  void set_ctime(time_t ctime) {
    _ctime = ctime;
  }

  void set_atime(time_t atime) {
    _atime = atime;
  }

  void set_mtime(time_t mtime) {
    _mtime = mtime;
  }

  void set_dev(dev_t dev) {
    _dev = dev;
  }

  static inline bool is_directory(mode_t mode) {
    return (mode & S_IFMT) == S_IFDIR;
  }

  static inline bool is_regular_file(mode_t mode) {
    return (mode & S_IFMT) == S_IFREG;
  }

  static inline bool is_character_device(mode_t mode) {
    return (mode & S_IFMT) == S_IFCHR;
  }

  static inline bool is_block_device(mode_t mode) {
    return (mode & S_IFMT) == S_IFBLK;
  }

  static inline bool is_fifo(mode_t mode) {
    return (mode & S_IFMT) == S_IFIFO;
  }

  static inline bool is_symlink(mode_t mode) {
    return (mode & S_IFMT) == S_IFLNK;
  }

  static inline bool is_socket(mode_t mode) {
    return (mode & S_IFMT) == S_IFSOCK;
  }

 protected:
  const uint32_t _index;
  off_t _size;
  mode_t _mode;
  uid_t _uid;
  gid_t _gid;
  time_t _ctime;  // create time
  time_t _atime;  // last access time
  time_t _mtime;  // last modification time
  dev_t _dev;     // 32 bits in total, 12 major, 20 minor.
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VNODE_H_
