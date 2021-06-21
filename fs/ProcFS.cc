// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/ProcFS.h>

#include <Algorithm.h>
#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <fs/Stat.h>
#include <fs/VirtualFileSystem.h>
#include <libs/CString.h>

namespace valkyrie::kernel {

namespace {

bool is_switch_on = false;

auto all_digits = [](const String& s) {
  return !s.empty() &&
    find_if(s.begin(), s.end(), [](char c) { return !is_digit(c); }) == s.end();
};

}  // namespace


ProcFS::ProcFS()
    : _next_inode_index(1),
      _root_inode(make_shared<ProcFSInode>(*this, nullptr, "/", S_IFDIR)) {
  // Lab 6: elective 3.1
  // The procfs creates switch and hello file in its root directory.
  // Users can access them by open, read, and write.
  _root_inode->add_child(make_shared<SwitchInode>(*this));
  _root_inode->add_child(make_shared<HelloInode>(*this));
}


SharedPtr<Vnode> ProcFS::get_root_vnode() {
  return _root_inode;
}

void ProcFS::repopulate_task_directories() {
  // Remove the task directories for tasks that no longer exist.
  _root_inode->_children.remove_if([](const auto& inode) {
    Task* task = nullptr;
    return all_digits(inode->get_name()) &&
           !(task = Task::get_by_pid(atoi(inode->get_name().c_str())));
  });

  for (auto task : Task::get_active_tasks()) {
    auto it = _root_inode->_children.find_if([&task](const auto& inode) {
      pid_t pid;
      return all_digits(inode->get_name()) &&
             (pid = atoi(inode->get_name().c_str())) == task->get_pid();
    });

    char pid_str[32] = {};
    sprintf(pid_str, "%d", task->get_pid());

    if (it == _root_inode->_children.end()) {
      auto task_dir = make_shared<ProcFSInode>(*this, _root_inode, pid_str, S_IFDIR);
      task_dir->add_child(make_shared<TaskStatusInode>(*this, task_dir));
      _root_inode->add_child(task_dir);
    }
  }
}


ProcFSInode::ProcFSInode(ProcFS& fs,
                         SharedPtr<ProcFSInode> parent,
                         const String& name,
                         mode_t mode)
    : Vnode(fs._next_inode_index++, 0, mode, 0, 0),
      _fs(fs),
      _name(name),
      _content(),
      _parent(parent),
      _children() {}


SharedPtr<Vnode> ProcFSInode::create_child(const String& name,
                                           const char* content,
                                           off_t size,
                                           mode_t mode,
                                           uid_t uid,
                                           gid_t gid) {
  Kernel::panic("ProcFS: invalid version of `create_child()` called: %s\n", name.c_str());
  /*
  auto inode = make_shared<ProcFSInode>(_fs, shared_from_this(), name, mode, type);
  _children.push_back(inode);
  return inode;
  */
}

void ProcFSInode::add_child(SharedPtr<Vnode> child) {
  _children.push_back(move(static_pointer_cast<ProcFSInode>(child)));
}

SharedPtr<Vnode> ProcFSInode::remove_child(const String& name) {
  SharedPtr<Vnode> removed_child;

  _children.remove_if([&removed_child, &name](auto& vnode) {
    return vnode->_name == name &&
           (removed_child = move(vnode), true);
  });

  if (!removed_child) [[unlikely]] {
    printk("ProcFS: <warning> unable to remove %s from %s\n", name, _name);
  }

  return removed_child;
}

SharedPtr<Vnode> ProcFSInode::get_child(const String& name) {
  if (name == ".") {
    return shared_from_this();
  } else if (name == "..") {
    return get_parent();
  }

  SharedPtr<Vnode> ret;

  // If we are trying to get the child of the root inode,
  // and all the chars in `name` are digits, then
  // we have to check whether the task with pid = `name` exists.
  if (is_root_vnode() && all_digits(name)) {
    auto it = _children.find_if([&name](auto& vnode) {
      return vnode->_name == name;
    });

    Task* task = Task::get_by_pid(atoi(name.c_str()));

    if (!task) [[unlikely]] {
      if (it != _children.end()) {
        remove_child(name);
      }
      return nullptr;
    }

    if (it != _children.end()) {
      ret = *it;
    } else {
      // Create task dir if it doesnt exist.
      // Also create the child (e.g., /proc/1/status) within the task dir.
      // The content of a child are updated only when the user tries to read them.
      auto task_dir = make_shared<ProcFSInode>(_fs, _fs._root_inode, name, S_IFDIR);
      task_dir->add_child(make_shared<TaskStatusInode>(_fs, task_dir));
      add_child(task_dir);

      ret = task_dir;
    }

  } else {
    auto it = _children.find_if([&name](const auto& vnode) {
      return vnode->_name == name;
    });

    ret = (it != _children.end()) ? *it : nullptr;
  }

  return ret;
}

SharedPtr<Vnode> ProcFSInode::get_ith_child(size_t i) {
  if (i == 0) {
    return shared_from_this();
  } else if (i == 1) {
    return get_parent();
  }

  for (auto it = _children.begin(); it != _children.end(); it++) {
    if (NR_SPECIAL_ENTRIES + it.index() == i) {
      return *it;
    }
  }
  return nullptr;
}

size_t ProcFSInode::get_children_count() const {
  _fs.repopulate_task_directories();
  return NR_SPECIAL_ENTRIES + _children.size();
}

SharedPtr<Vnode> ProcFSInode::get_parent() {
  if (is_root_vnode()) {
    return VFS::get_instance().get_host_vnode(_fs._root_inode)->get_parent();
  }

  return _parent.lock();
}

void ProcFSInode::set_parent(SharedPtr<Vnode> parent) {
  _parent = parent;
}


int ProcFSInode::chmod(const mode_t mode) {
  return -1;
}

int ProcFSInode::chown(const uid_t uid, const gid_t gid) {
  return -1;
}

String ProcFSInode::get_name() const {
  return _name;
}

char* ProcFSInode::get_content() {
  printk("ProcFS: ProcFSInode::get_content() called...\n");
  return nullptr;
}

void ProcFSInode::set_content(UniquePtr<char[]> content, off_t) {
  printk("ProcFS: ProcFSInode::set_content() called...\n");
}

size_t ProcFSInode::hash_code() const {
  return Hash<ProcFSInode>{}(*this);
}

bool ProcFSInode::is_root_vnode() const {
  return this == _fs._root_inode.get();
}


char* SwitchInode::get_content() {
  constexpr size_t len = 2;
  _content = make_unique<char[]>(len);

  if (is_switch_on) {
    _content[0] = '1';
  } else {
    _content[0] = '0';
  }
  _content[1] = '\n';

  _size = len;
  return _content.get();
}

void SwitchInode::set_content(UniquePtr<char[]> content, off_t new_size) {
 is_switch_on = static_cast<bool>(atoi(content.get()));
}


char* HelloInode::get_content() {
  constexpr size_t len = 6;
  _content = make_unique<char[]>(len);

  if (is_switch_on) {
    strncpy(_content.get(), "HELLO\n", len);
  } else {
    strncpy(_content.get(), "hello\n", len);
  }

  _size = len;
  return _content.get();
}

char* TaskStatusInode::get_content() {
  pid_t pid = 0;
  Task* task = nullptr;

  if (auto parent = _parent.lock()) [[likely]] {
    pid = atoi(parent->get_name().c_str());
  } else {
    printk("ProcFS: <warning> WeakPtr::lock() failed on _parent.\n");
    return nullptr;
  }

  task = Task::get_by_pid(pid);

  if (!task) [[unlikely]] {
    printk("ProcFS: <warning> task dir exists but cannot get task object!?\n");
    return nullptr;
  }

  constexpr size_t len = 128;
  _content = make_unique<char[]>(len);

  sprintf(_content.get(),
          "Name: %s\n"
          "State: %d\n"
          "Pid: %d\n",
          task->get_name(), task->get_state(), task->get_pid());

  _size = len;
  return _content.get();
}

}  // namespace valkyrie::kernel
