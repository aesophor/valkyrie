// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <SlobAllocator.h>

#include <Console.h>
#include <Kernel.h>
#include <Math.h>

namespace valkyrie::kernel {

SlobAllocator::SlobAllocator(PageFrameAllocator* page_frame_allocator)
    : _page_frame_allocator(page_frame_allocator),
      _bins(),
      _top_chunk(),
      _top_chunk_end() {}


void* SlobAllocator::allocate(size_t requested_size) {
  if (!requested_size) {
    return nullptr;
  }

  requested_size = sanitize_size(requested_size + sizeof(Slob));
  int index = get_bin_index(requested_size);

  // Search for an exact-fit free chunk from the corresponding bin.
  Slob* victim = _bins[index];
  if (victim) {
    printf("_bins[%d] hit!\n", index);
    bin_del_head(victim);
    dump_slob_info();
    return victim + 1;
  }

  printf("no existing free chunk available\n");

  // Search larger free chunks, and attempt to split an exact-fit chunk
  // for a larger free chunk.
  for (; index < NUM_OF_BINS; index++) {
    if (_bins[index]) {
      victim = _bins[index];
      printf("found a larger chunk at _bins[%d]\n", index);
      break;
    }
  }

  if (victim) {
    victim = split_chunk(victim, requested_size);
    return victim + 1;
  }

  printf("no larger free chunk to split from. Splitting from the top chunk.\n");

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
  dump_slob_info();
  return victim + 1;  // skip the header
}

void SlobAllocator::deallocate(void* p) {
  if (!p) {
    return;
  }

  printf("deallocating... 0x%x\n", p);
  Slob* slob = reinterpret_cast<Slob*>(p) - 1;  // 1 is for the header
  printf("slob order: %d\n", slob->index);
  //slob->flags = SLOB_FLAG_FREE;
  bin_add_head(slob);

  // Attempt to merge this chunk with its next one.
  /*
  size_t ptr = reinterpret_cast<size_t>(slob);
  ptr -= slob->prev_size;
  Slob* prev_slob = reinterpret_cast<Slob*>(ptr);

  if (!prev_slob->is_allocated()) {

  }
  */

  // Attempt to merge this chunk with its previous one.


  dump_slob_info();
}

void SlobAllocator::dump_slob_info() const {
  puts("--- dumping slob bins ---");

  for (int i = 0; i < NUM_OF_BINS; i++) {
    printf("_bins[%d]: ", i);
    Slob* ptr = _bins[i];
    while (ptr) {
      printf("[%d] -> ", ptr->index);
      ptr = ptr->next;
    }
    printf("(null)\n");
  }

  puts("--- end dumping slob bins ---");
}


void SlobAllocator::request_new_page_frame() {
  _top_chunk = _page_frame_allocator->allocate_one_page_frame();
  _top_chunk_end = reinterpret_cast<char*>(_top_chunk) + PAGE_SIZE;

  printf("_top_chunk     = 0x%x\n", _top_chunk);
  printf("_top_chunk_end = 0x%x\n", _top_chunk_end);
}

SlobAllocator::Slob* SlobAllocator::split_from_top_chunk(size_t requested_size) {
  if (!requested_size) {
    return nullptr;
  }

  Slob* chunk = reinterpret_cast<Slob*>(_top_chunk);
  chunk->next = nullptr;
  chunk->index = get_bin_index(requested_size);

  _top_chunk = reinterpret_cast<char*>(_top_chunk) + requested_size;
  return chunk;
}

bool SlobAllocator::is_top_chunk_used_up() const {
  if (_top_chunk > _top_chunk_end) {
    Kernel::panic("kernel heap corrupted (_top_chunk > _top_chunk_end)\n");
  }
  return get_top_chunk_size() == 0;
}

bool SlobAllocator::is_top_chunk_large_enough(const size_t requested_size) const {
  return get_top_chunk_size() >= requested_size;
}

size_t SlobAllocator::get_top_chunk_size() const {
  return reinterpret_cast<size_t>(_top_chunk_end) -
         reinterpret_cast<size_t>(_top_chunk);
}

SlobAllocator::Slob* SlobAllocator::split_chunk(Slob* chunk,
                                                const size_t target_size) {
  if (!chunk) {
    Kernel::panic("kernel heap corrupted (chunk == nullptr)\n");
  }

  if (chunk->index < 0 || chunk->index >= NUM_OF_BINS) {
    Kernel::panic("kernel heap corrupted (invalid chunk->index: %d)\n", chunk->index);
  }

  bin_del_head(chunk);

  // Update chunk headers
  size_t remainder_size = get_chunk_size(chunk->index) - target_size;
  size_t remainder_addr = reinterpret_cast<size_t>(chunk) + target_size;

  Slob* remainder = reinterpret_cast<Slob*>(remainder_addr);
  remainder->next = nullptr;
  remainder->index = get_bin_index(remainder_size);

  chunk->next = nullptr;
  chunk->index = get_bin_index(target_size);

  // Add remainder chunk to the corresponding bin.
  bin_add_head(remainder);

  return chunk;
}


void SlobAllocator::bin_del_head(Slob* chunk) {
  if (!chunk) {
    Kernel::panic("kernel heap corrupted: bin_del_head(nullptr)\n");
  }

  if (!chunk || !_bins[chunk->index]) {
    return;
  }

  _bins[chunk->index] = _bins[chunk->index]->next;
  chunk->next = nullptr;
}

void SlobAllocator::bin_add_head(Slob* chunk) {
  if (!chunk) {
    Kernel::panic("kernel heap corrupted: bin_add_head(nullptr)\n");
  }

  if (_bins[chunk->index] == chunk) {
    return;
  }

  // If the list is empty
  if (!_bins[chunk->index]) {
    _bins[chunk->index] = chunk;
    chunk->next = nullptr;
  } else {
    chunk->next = _bins[chunk->index];
    _bins[chunk->index] = chunk;
  }
}


int SlobAllocator::get_bin_index(size_t size) {
  int ret = (size - CHUNK_SMALLEST_SIZE) / CHUNK_SIZE_GAP;
  printf("get_bin_index(%d) = %d\n", size, ret);
  return ret;
}

size_t SlobAllocator::get_chunk_size(const int index) {
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
    result <<= 1;
  }
  return result;
}

}  // namespace valkyrie::kernel
