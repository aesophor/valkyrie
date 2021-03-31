// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <SlobAllocator.h>

#include <Console.h>
#include <Kernel.h>
#include <Math.h>

namespace valkyrie::kernel {

SlobAllocator::SlobAllocator(PageFrameAllocator* page_frame_allocator)
    : _page_frame_allocator(page_frame_allocator),
      _page_frame_begin(),
      _top_chunk(),
      _page_frame_end(),
      _bins(),
      _unsorted_bin() {}


void* SlobAllocator::allocate(size_t requested_size) {
  if (!requested_size) {
    return nullptr;
  }

  Slob* victim = nullptr;
  requested_size = sanitize_size(requested_size + sizeof(Slob));
  int index = get_bin_index(requested_size);

  // Search for an exact-fit free chunk from the corresponding bin.
  if (index < NUM_OF_BINS && (victim = _bins[index])) {
    printf("_bins[%d] hit!\n", index);
    victim->set_allocated(true);
    bin_del_head(victim);
    goto out;
  }

  // Search larger free chunks from the unsorted bin.
  printf("no existing free chunk available. Searching from the unsorted bin.\n");
  for (Slob* chunk = _unsorted_bin; chunk; chunk = chunk->next) {
    if (get_chunk_size(chunk->index) >= requested_size) {
      printf("found a larger chunk from _unsorted_bin\n");
      victim = split_chunk(chunk, requested_size);
      goto out;
    }
  }

  // Search larger free chunks, and attempt to split an exact-fit chunk
  // for a larger free chunk.
  printf("no suitable free chunk from unsorted bin. Searching from regular bins.\n");
  for (; index < NUM_OF_BINS; index++) {
    if (_bins[index]) {
      printf("found a larger chunk at _bins[%d]\n", index);
      victim = split_chunk(_bins[index], requested_size);
      goto out;
    }
  }

  printf("no larger free chunk from regular bins. Splitting from the top chunk.\n");

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
  dump_slob_info();
  return victim + 1;  // skip the header
}

void SlobAllocator::deallocate(void* p) {
  if (!p) {
    return;
  }

  printf("deallocating... 0x%x\n", p);

  Slob* chunk = reinterpret_cast<Slob*>(p) - 1;  // 1 is for the header
  size_t chunk_addr = reinterpret_cast<size_t>(chunk);

  // Maybe merge this chunk with the top chunk.
  size_t next_chunk_addr = chunk_addr + get_chunk_size(chunk->index);
  Slob* next_chunk = reinterpret_cast<Slob*>(next_chunk_addr);

  if (next_chunk == _top_chunk) {
    printf("merging with the top chunk\n");
    _top_chunk = chunk;
  } else {
    bin_add_head(chunk);
  }

  // Maybe merge this chunk with its previous ones.
  size_t prev_chunk_addr = chunk_addr - chunk->get_prev_chunk_size();
  Slob* prev_chunk = reinterpret_cast<Slob*>(prev_chunk_addr);

  if (!prev_chunk->is_allocated() && !is_first_chunk_in_page_frame(chunk)) {
    printf("omg!!!\n");
    bin_del_entry(chunk);
    bin_del_entry(prev_chunk);
    size_t prev_chunk_size = get_chunk_size(prev_chunk->index);
    size_t curr_chunk_size = get_chunk_size(chunk->index);
    prev_chunk->index = get_bin_index(prev_chunk_size + curr_chunk_size);
    bin_add_head(prev_chunk);
  }

  dump_slob_info();
}

void SlobAllocator::dump_slob_info() const {
  puts("--- dumping slob bins ---");

  Slob* ptr = nullptr;

  for (int i = 0; i < NUM_OF_BINS; i++) {
    printf("_bins[%d] (%d): ", i, get_chunk_size(i));
    ptr = _bins[i];
    while (ptr) {
      printf("[0x%x] -> ", ptr);
      ptr = ptr->next;
    }
    printf("(null)\n");
  }

  printf("_unsorted_bin: ");
  ptr = _unsorted_bin;
  while (ptr) {
    printf("[%d 0x%x] -> ", get_chunk_size(ptr->index), ptr);
    ptr = ptr->next;
  }
  printf("(null)\n");

  printf("_page_frame_begin = 0x%x\n", _page_frame_begin);
  printf("_top_chunk        = 0x%x\n", _top_chunk);
  printf("_page_frame_end   = 0x%x\n", _page_frame_end);

  puts("--- end dumping slob bins ---");
}


void SlobAllocator::request_new_page_frame() {
  _page_frame_begin = _page_frame_allocator->allocate_one_page_frame();

  _top_chunk = _page_frame_begin;

  _page_frame_end = reinterpret_cast<char*>(_top_chunk) +
                    PAGE_SIZE -
                    PageFrameAllocator::get_block_header_size();
}

bool SlobAllocator::is_first_chunk_in_page_frame(const Slob* chunk) const {
  size_t chunk_addr = reinterpret_cast<size_t>(chunk);
  size_t page_frame_begin_addr = reinterpret_cast<size_t>(_page_frame_begin);
  return chunk_addr == page_frame_begin_addr + sizeof(Slob);
}

SlobAllocator::Slob* SlobAllocator::split_from_top_chunk(size_t requested_size) {
  static int32_t prev_chunk_size = 0;

  if (!requested_size) {
    return nullptr;
  }

  Slob* chunk = reinterpret_cast<Slob*>(_top_chunk);
  chunk->next = nullptr;
  chunk->index = get_bin_index(requested_size);
  chunk->prev_chunk_size = prev_chunk_size;
  chunk->set_allocated(true);

  prev_chunk_size = requested_size;

  _top_chunk = reinterpret_cast<char*>(_top_chunk) + requested_size;
  return chunk;
}

bool SlobAllocator::is_top_chunk_used_up() const {
  if (_top_chunk > _page_frame_end) {
    Kernel::panic("kernel heap corrupted (_top_chunk > _top_chunk_end)\n");
  }
  return get_top_chunk_size() == 0;
}

bool SlobAllocator::is_top_chunk_large_enough(const size_t requested_size) const {
  return get_top_chunk_size() >= requested_size;
}

size_t SlobAllocator::get_top_chunk_size() const {
  return reinterpret_cast<size_t>(_page_frame_end) -
         reinterpret_cast<size_t>(_top_chunk);
}

SlobAllocator::Slob* SlobAllocator::split_chunk(Slob* chunk,
                                                const size_t target_size) {
  if (!chunk) {
    Kernel::panic("kernel heap corrupted (chunk == nullptr)\n");
  }

  if (chunk->index < 0) {
    Kernel::panic("kernel heap corrupted (invalid chunk->index: %d)\n", chunk->index);
  }

  bin_del_head(chunk);

  // Update chunk headers
  size_t remainder_size = get_chunk_size(chunk->index) - target_size;

  if (remainder_size > 0) {
    size_t remainder_addr = reinterpret_cast<size_t>(chunk) + target_size;
    Slob* remainder = reinterpret_cast<Slob*>(remainder_addr);

    remainder->next = nullptr;
    remainder->index = get_bin_index(remainder_size);
    remainder->prev_chunk_size = target_size;
    remainder->set_allocated(false);

    // Add the remainder chunk to the corresponding bin.
    printf("putting the remainder to bin\n");
    bin_add_head(remainder);
  }

  chunk->next = nullptr;
  chunk->index = get_bin_index(target_size);
  chunk->set_allocated(true);
  return chunk;
}


void SlobAllocator::bin_del_head(Slob* chunk) {
  if (!chunk) {
    Kernel::panic("kernel heap corrupted: bin_del_head(nullptr)\n");
  }

  // If `chunk` doesn't fit in regular bins,
  // we should use bin_del_entry() to remove it from the unsorted bin.
  if (chunk->index >= NUM_OF_BINS) {
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
  if (!chunk) {
    Kernel::panic("kernel heap corrupted: bin_add_head(nullptr)\n");
  }

  // If `chunk->index` >= NUM_OF_BINS,
  // then we add this chunk to the head of _unsorted_bin.
  Slob** bin_first_chunk = (chunk->index >= NUM_OF_BINS) ? &_unsorted_bin
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
  if (!chunk) {
    Kernel::panic("kernel heap corrupted: bin_del_entry(nullptr)\n");
  }

  // If `chunk->index` >= NUM_OF_BINS,
  // then need to look for `chunk` from the unsorted bin.
  Slob** bin_first_chunk = (chunk->index >= NUM_OF_BINS) ? &_unsorted_bin
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
  int ret = (size - CHUNK_SMALLEST_SIZE) / CHUNK_SIZE_GAP;
  printf("get_bin_index(%d) = %d\n", size, ret);
  return ret;
}

size_t SlobAllocator::get_chunk_size(const int index) const {
  return CHUNK_SMALLEST_SIZE + index * CHUNK_SIZE_GAP;
}


size_t SlobAllocator::sanitize_size(size_t size) {
  if (size < CHUNK_SMALLEST_SIZE) {
    size = CHUNK_SMALLEST_SIZE;
  }

  return round_up_to_multiple_of_16(size);
}

size_t SlobAllocator::round_up_to_multiple_of_16(size_t x) {
  size_t result = 16;
  while (result < x) {
    result += 16;
  }
  return result;
}


int32_t SlobAllocator::Slob::get_prev_chunk_size() const {
  return prev_chunk_size & ~1;
}

bool SlobAllocator::Slob::is_allocated() const {
  return static_cast<bool>(prev_chunk_size & 1);
}

void SlobAllocator::Slob::set_allocated(bool allocated) {
  if (allocated) {
    prev_chunk_size |= 1;
  } else {
    prev_chunk_size &= ~1;
  }
}

}  // namespace valkyrie::kernel
