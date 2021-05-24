// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <dev/StorageDevice.h>

#include <Memory.h>
#include <dev/MasterBootRecord.h>

namespace valkyrie::kernel {

StorageDevice::StorageDevice(BlockDeviceDriver& driver,
                             size_t block_size)
    : BlockDevice(driver, block_size),
      _partitions() {

  auto buffer = make_unique<char[]>(block_size);
 
  // If the storage device's 0th sector is the MBR,
  // then we can parse it to get each partition's type, size, and block index.
  read_block(0, buffer.get());
  const MBR* mbr = reinterpret_cast<MBR*>(buffer.get());

  for (int i = 0; i < NR_MAX_PARTITIONS; i++) {
    auto partition = reinterpret_cast<const MBR::PartitionMetadata*>(&mbr->partitions[i]);

    uint32_t start_block_index;
    memcpy(&start_block_index, &partition->lba_addr_partition_start, sizeof(start_block_index));

    if (!start_block_index) {
      continue;
    }

    uint32_t end_block_index;
    memcpy(&end_block_index, &partition->nr_sectors, sizeof(end_block_index));
    end_block_index += start_block_index - 1;

    _partitions.push_back(
        make_unique<DiskPartition>(*this, start_block_index, end_block_index, String("sda") + "1"));

    //printk("partition [%d]: begin = 0x%x, size = 0x%x\n", i, start_block_index, mbr->partitions[i].nr_sectors);
  }

}


void StorageDevice::read_block(int block_index, void* buf) {
  _driver.read_block(block_index, buf);
}

void StorageDevice::write_block(int block_index, const void* buf) {
  _driver.write_block(block_index, buf);
}


DiskPartition& StorageDevice::get_first_partition() const {
  return *_partitions.front();
}

}  // namespace valkyrie::kernel
