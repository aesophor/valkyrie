// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_VNODE_H_
#define VALKYRIE_VNODE_H_

#include <Types.h>
#include <Memory.h>
#include <String.h>
#include <fs/Stat.h>

namespace valkyrie::kernel {

// Forward declaration
class FileSystem;

class Vnode {
 public:
  Vnode(const uint32_t index,
        mode_t mode,
        uid_t uid,
        gid_t gid)
      : _index(index),
        _mode(mode),
        _uid(uid),
        _gid(gid) {}

  virtual ~Vnode() = default;

  
  virtual SharedPtr<Vnode> create_child(const String& name,
                                        const char* content,
                                        size_t size,
                                        mode_t mode,
                                        uid_t uid,
                                        gid_t gid) = 0;
  virtual void add_child(SharedPtr<Vnode> child) = 0;
  virtual SharedPtr<Vnode> remove_child(const String& name) = 0;
  virtual SharedPtr<Vnode> get_child(const String& name) = 0;

  virtual int chmod(const mode_t mode) = 0;
  virtual int chown(const uid_t uid, const gid_t gid) = 0;

  virtual char* get_content() const = 0;
  virtual void set_content(UniquePtr<char[]> content) = 0;


  bool is_directory() const { return (_mode & S_IFMT) == S_IFDIR; }
  bool is_regular_file() const { return (_mode & S_IFMT) == S_IFREG; }
  bool is_character_device() const { return (_mode & S_IFMT) == S_IFCHR; }
  bool is_block_device() const { return (_mode & S_IFMT) == S_IFBLK; }
  bool is_fifo() const { return (_mode & S_IFMT) == S_IFIFO; }
  bool is_symlink() const { return (_mode & S_IFMT) == S_IFLNK; }
  bool is_socket() const { return (_mode & S_IFMT) == S_IFSOCK; }

  bool is_sticky() const { return _mode & S_ISVTX; }
  bool is_setuid() const { return _mode & S_ISUID; }
  bool is_setgid() const { return _mode & S_ISGID; }

  uint32_t get_index() const { return _index; }
  off_t get_size() const { return _size; }
  mode_t get_mode() const { return _mode; }
  time_t get_ctime() const { return _ctime; }
  time_t get_atime() const { return _atime; }
  time_t get_mtime() const { return _mtime; }

  void set_size(off_t size) { _size = size; }
  void set_ctime(time_t ctime) { _ctime = ctime; }
  void set_atime(time_t atime) { _atime = atime; }
  void set_mtime(time_t mtime) { _mtime = mtime; }


 protected:
  const uint32_t _index;
  off_t _size;
  mode_t _mode;
  uid_t _uid;
  gid_t _gid;
  time_t _ctime;  // create time 
  time_t _atime;  // last access time
  time_t _mtime;  // last modification time
  dev_t _dev_number;  // 32 bits in total, 12 major, 20 minor.
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_VNODE_H_
