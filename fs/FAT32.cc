// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <fs/FAT32.h>

#include <dev/SDCardDriver.h>

namespace valkyrie::kernel {

FAT32Vnode::FAT32Vnode(FAT32& fs,
                       FAT32Vnode* parent,
                       const String& name,
                       mode_t mode,
                       uid_t uid,
                       gid_t gid)
    : Vnode(fs._next_vnode_index++, mode, uid, gid),
      _fs(fs),
      _name(name),
      _parent(parent),
      _children() {}


FAT32::FAT32()
    : _next_vnode_index(0),
      _root_vnode() {
  auto& sdcard_driver = SDCardDriver::get_instance();
  
  char buf[512];
  sdcard_driver.read_block(0, buf);
  for (auto& c : buf) {
    printf("%c ", c);
  }
  printf("\n");

  const BootSector* bpb = reinterpret_cast<BootSector*>(buf);
  printk("BS_OEMName: %s\n", bpb->oem_name);
}


SharedPtr<Vnode> FAT32::get_root_vnode() {
  return _root_vnode;
}

}  // namespace valkyrie::kernel
