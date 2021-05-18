// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_MASTER_BOOT_RECORD_H_
#define VALKYRIE_MASTER_BOOT_RECORD_H_

#include <List.h>
#include <Memory.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>

#define NR_MAX_PARTITIONS  4

namespace valkyrie::kernel {

struct [[gnu::packed]] MBR final {
  uint8_t bootstrap[440];
  uint32_t unique_disk_id;
  uint16_t __reserved;

  struct [[gnu::packed]] PartitionMetadata final {
    uint8_t drive_attributes;
    uint8_t chs_addr_partition_start[3];
    uint8_t partition_type;
    uint8_t chs_addr_last_parition_sector[3];
    uint32_t lba_addr_partition_start;
    uint32_t nr_sectors;  // in this partition
  } partitions[NR_MAX_PARTITIONS];

  uint16_t signature;
};


static_assert(sizeof(MBR) == 512);
static_assert(sizeof(MBR::PartitionMetadata) == 16);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_MASTER_BOOT_RECORD_H_
