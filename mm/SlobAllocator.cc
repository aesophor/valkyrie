// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/SlobAllocator.h>

#include <Algorithm.h>
#include <dev/Console.h>
#include <kernel/Compiler.h>
#include <kernel/Kernel.h>
#include <libs/Math.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

SlobAllocator::SlobAllocator(BuddyAllocator* page_frame_allocator)
    : _buddy_allocator(page_frame_allocator),
      _page_frame_allocatable_begin(),
      _top_chunk(),
      _page_frame_allocatable_end(),
      _top_chunk_prev_chunk_size(),
      _bins(),
      _unsorted_bin() {}


void* SlobAllocator::allocate(size_t requested_size) {
  if (unlikely(!requested_size)) {
    return nullptr;
  }

  requested_size = normalize_size(requested_size + sizeof(Slob));
  int index = get_bin_index(requested_size);

  Slob* victim = nullptr;

  // Search for an exact-fit free chunk from the corresponding bin.
  if (index < NR_BINS && (victim = _bins[index])) {
    victim->set_allocated(true);
    bin_del_head(victim);
    goto out;
  }

  // Search larger free chunks from the unsorted bin.
  for (Slob* chunk = _unsorted_bin; chunk; chunk = chunk->next) {
    if (chunk->get_chunk_size() >= requested_size) {
      victim = split_chunk(chunk, requested_size);
      goto out;
    }
  }

  // Search larger free chunks, and attempt to split an exact-fit chunk
  // from a larger free chunk.
  for (; index < NR_BINS; index++) {
    if (_bins[index]) {
      victim = split_chunk(_bins[index], requested_size);
      goto out;
    }
  }

  if (is_top_chunk_used_up()) {
    request_new_page_frame();
  } else if (!is_top_chunk_large_enough(requested_size)) {
    // The top chunk hasn't been used up yet, but
    // it is not large enough, so we could put it into the corresponding bin,
    // and finally request a new page frame.
    bin_add_head(split_from_top_chunk(get_top_chunk_size()));
    request_new_page_frame();
  } 

  victim = split_from_top_chunk(requested_size);

out:
  return victim + 1;  // skip the header
}

void SlobAllocator::deallocate(void* p) {
  if (unlikely(!p)) {
    return;
  }

  Slob* mid_chunk = reinterpret_cast<Slob*>(p) - 1;  // 1 is for the header
  size_t mid_chunk_addr = reinterpret_cast<size_t>(mid_chunk);
  size_t mid_chunk_size = mid_chunk->get_chunk_size();

  size_t prev_chunk_addr = mid_chunk_addr - mid_chunk->get_prev_chunk_size();
  Slob* prev_chunk = reinterpret_cast<Slob*>(prev_chunk_addr);
  size_t prev_chunk_size = mid_chunk->get_prev_chunk_size();

  size_t next_chunk_addr = mid_chunk_addr + mid_chunk->get_chunk_size();
  Slob* next_chunk = reinterpret_cast<Slob*>(next_chunk_addr);
  size_t next_chunk_size = next_chunk->get_chunk_size();

  // Final chunk pointer and size (after being merged)
  Slob* chunk = mid_chunk;
  size_t chunk_size = mid_chunk_size;

  // Mark current chunk as unallocated.
  mid_chunk->set_allocated(false);

  // Maybe merge this chunk with its previous one.
  if (!prev_chunk->is_allocated() && !is_first_chunk_in_page_frame(mid_chunk)) {
    bin_del_entry(prev_chunk);
    chunk_size += prev_chunk_size;
    prev_chunk->next = next_chunk;
    prev_chunk->index = get_bin_index(chunk_size);
    next_chunk->set_prev_chunk_size(chunk_size);
    chunk = prev_chunk;
  }

  // Maybe merge this chunk with its next one.
  if (next_chunk == _top_chunk) {
    // The next one is the top chunk.
    _top_chunk = chunk;
    _top_chunk_prev_chunk_size = chunk->get_prev_chunk_size();
    return;
  } if (!next_chunk->is_allocated()) {
    // The next one is a regular freed chunk.
    bin_del_entry(next_chunk);
    chunk_size += next_chunk_size;
    chunk->next = reinterpret_cast<Slob*>(next_chunk_addr + next_chunk_size);
    chunk->index = get_bin_index(chunk_size);
    chunk->next->set_prev_chunk_size(chunk_size);
  }

  // Put the merged chunk to the bin.
  bin_add_head(chunk);
}

void SlobAllocator::dump_slob_info() const {
  puts("--- dumping slob bins ---");

  Slob* ptr = nullptr;

  for (int i = 0; i < NR_BINS; i++) {
    printf("_bins[%d] (%d): ", i, CHUNK_SMALLEST_SIZE + CHUNK_SIZE_GAP * i);
    ptr = _bins[i];
    while (ptr) {
      printf("[%d 0x%x] -> ", ptr->get_chunk_size(), ptr);
      ptr = ptr->next;
    }
    printf("(null)\n");
  }

  printf("_unsorted_bin: ");
  ptr = _unsorted_bin;
  while (ptr) {
    printf("[%d 0x%x] -> ", ptr->get_chunk_size(), ptr);
    ptr = ptr->next;
  }
  printf("(null)\n");
  puts("--- end dumping slob bins ---");

  printf("_page_frame_allocatable_begin = 0x%x\n", _page_frame_allocatable_begin);
  printf("_top_chunk                    = 0x%x\n", _top_chunk);
  printf("_page_frame_allocatable_end   = 0x%x\n", _page_frame_allocatable_end);

  if (unlikely(_top_chunk > _page_frame_allocatable_end)) {
    Kernel::panic("kernel heap corrupted "
                  "(_top_chunk > _page_frame_allocatable_end)\n");
  }
}

size_t SlobAllocator::get_chunk_header_size() {
  return sizeof(SlobAllocator::Slob);
}


void SlobAllocator::request_new_page_frame() {
  _page_frame_allocatable_begin = _buddy_allocator->allocate_one_page_frame();

  _top_chunk = _page_frame_allocatable_begin;

  _page_frame_allocatable_end = reinterpret_cast<char*>(_top_chunk) +
                                PAGE_SIZE -
                                BuddyAllocator::get_block_header_size();
}

bool SlobAllocator::is_first_chunk_in_page_frame(const Slob* chunk) const {
  size_t chunk_addr = reinterpret_cast<size_t>(chunk);
  size_t page_frame_allocatable_begin_addr
    = reinterpret_cast<size_t>(_page_frame_allocatable_begin);
  return chunk_addr == page_frame_allocatable_begin_addr;
}

SlobAllocator::Slob* SlobAllocator::split_from_top_chunk(size_t requested_size) {
  if (!requested_size) {
    return nullptr;
  }

  Slob* chunk = reinterpret_cast<Slob*>(_top_chunk);
  chunk->next = nullptr;
  chunk->index = get_bin_index(requested_size);
  chunk->prev_chunk_size = _top_chunk_prev_chunk_size;
  chunk->set_allocated(true);

  _top_chunk = reinterpret_cast<char*>(_top_chunk) + requested_size;
  _top_chunk_prev_chunk_size = requested_size;

  return chunk;
}

bool SlobAllocator::is_top_chunk_used_up() const {
  return get_top_chunk_size() == 0;
}

bool SlobAllocator::is_top_chunk_large_enough(const size_t requested_size) const {
  return get_top_chunk_size() >= requested_size;
}

size_t SlobAllocator::get_top_chunk_size() const {
  return reinterpret_cast<size_t>(_page_frame_allocatable_end) -
         reinterpret_cast<size_t>(_top_chunk);
}

SlobAllocator::Slob* SlobAllocator::split_chunk(Slob* chunk,
                                                const size_t target_size) {
  if (unlikely(!chunk)) {
    Kernel::panic("kernel heap corrupted (chunk == nullptr)\n");
  }

  if (unlikely(chunk->index < 0)) {
    Kernel::panic("kernel heap corrupted (invalid chunk->index: %d)\n", chunk->index);
  }

  if (unlikely(target_size > chunk->get_chunk_size())) {
    Kernel::panic("kernel heap corrupted"
                  "(unable to split %d bytes from a %d byte chunk)",
                  target_size, chunk->get_chunk_size());
  }

  bin_del_head(chunk);

  // Update chunk headers
  size_t remainder_size = chunk->get_chunk_size() - target_size;

  if (likely(remainder_size > 0)) {
    size_t remainder_addr = reinterpret_cast<size_t>(chunk) + target_size;
    Slob* remainder = reinterpret_cast<Slob*>(remainder_addr);

    remainder->next = nullptr;
    remainder->index = get_bin_index(remainder_size);
    remainder->prev_chunk_size = target_size;
    remainder->set_allocated(false);

    // Put the remainder chunk to the corresponding bin.
    bin_add_head(remainder);

    // If the chunk after `chunk` isn't the top chunk,
    // then we need to update that chunk's prev_size.
    size_t next_chunk_addr = reinterpret_cast<size_t>(chunk) + chunk->get_chunk_size();
    Slob* next_chunk = reinterpret_cast<Slob*>(next_chunk_addr);

    if (next_chunk != _top_chunk) {
      next_chunk->set_prev_chunk_size(remainder_size);
    }
  }

  chunk->next = nullptr;
  chunk->index = get_bin_index(target_size);
  chunk->set_allocated(true);
  return chunk;
}


void SlobAllocator::bin_del_head(Slob* chunk) {
  if (unlikely(!chunk)) {
    Kernel::panic("kernel heap corrupted: bin_del_head(nullptr)\n");
  }

  // If `chunk` doesn't fit in regular bins,
  // we should use bin_del_entry() to remove it from the unsorted bin.
  if (chunk->index >= NR_BINS) {
    bin_del_entry(chunk);
    return;
  }

  // Try to remove the head element from the corresponding regular bin.
  if (!_bins[chunk->index]) {
    return;
  }

  _bins[chunk->index] = _bins[chunk->index]->next;
  chunk->next = nullptr;
}

void SlobAllocator::bin_add_head(Slob* chunk) {
  if (unlikely(!chunk)) {
    Kernel::panic("kernel heap corrupted: bin_add_head(nullptr)\n");
  }

  // If `chunk->index` >= NUM_OF_BINS,
  // then we add this chunk to the head of _unsorted_bin.
  Slob** bin_first_chunk = (chunk->index >= NR_BINS) ? &_unsorted_bin
                                                     : &_bins[chunk->index];

  if (*bin_first_chunk == chunk) {
    return;
  }

  // If the bin is empty.
  if (!*bin_first_chunk) {
    *bin_first_chunk = chunk;
    chunk->next = nullptr;
  } else {
    chunk->next = *bin_first_chunk;
    *bin_first_chunk = chunk;
  }
}

void SlobAllocator::bin_del_entry(Slob* chunk) {
  if (unlikely(!chunk)) {
    Kernel::panic("kernel heap corrupted: bin_del_entry(nullptr)\n");
  }

  // If `chunk->index` >= NUM_OF_BINS,
  // then need to look for `chunk` from the unsorted bin.
  Slob** bin_first_chunk = (chunk->index >= NR_BINS) ? &_unsorted_bin
                                                     : &_bins[chunk->index];

  if (*bin_first_chunk == chunk) {
    *bin_first_chunk = (*bin_first_chunk)->next;
    chunk->next = nullptr;
    return;
  }

  Slob* prev = nullptr;
  Slob* ptr = *bin_first_chunk;

  while (ptr) {
    if (ptr == chunk) {
      prev->next = ptr->next;
      ptr->next = nullptr;
      break;
    }

    prev = ptr;
    ptr = ptr->next;
  }
}


int SlobAllocator::get_bin_index(size_t size) {
  return (size - CHUNK_SMALLEST_SIZE) / CHUNK_SIZE_GAP;
}

size_t SlobAllocator::normalize_size(size_t size) {
  return round_up_to_multiple_of_n(max(size, CHUNK_SMALLEST_SIZE), 16);
}


size_t SlobAllocator::Slob::get_chunk_size() const {
  return CHUNK_SMALLEST_SIZE + index * CHUNK_SIZE_GAP;
}

int32_t SlobAllocator::Slob::get_prev_chunk_size() const {
  return prev_chunk_size & ~1;
}

void SlobAllocator::Slob::set_prev_chunk_size(const int32_t size) {
  const bool flag = is_allocated();
  prev_chunk_size = size;
  set_allocated(flag);
}

bool SlobAllocator::Slob::is_allocated() const {
  return prev_chunk_size & 1;
}

void SlobAllocator::Slob::set_allocated(bool allocated) {
  if (allocated) {
    prev_chunk_size |= 1;
  } else {
    prev_chunk_size &= ~1;
  }
}

}  // namespace valkyrie::kernel
