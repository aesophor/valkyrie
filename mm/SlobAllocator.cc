// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <mm/SlobAllocator.h>

#include <dev/Console.h>
#include <kernel/Kernel.h>
#include <mm/Page.h>

namespace valkyrie::kernel {

SlobAllocator::SlobAllocator(BuddyAllocator *page_frame_allocator)
    : _buddy_allocator(page_frame_allocator),
      _page_frame_allocatable_begin(),
      _top_chunk(),
      _page_frame_allocatable_end(),
      _top_chunk_prev_chunk_size(),
      _bins(),
      _unsorted_bin() {}

void *SlobAllocator::allocate(size_t requested_size) {
  if (!requested_size) [[unlikely]] {
    return nullptr;
  }

  requested_size = normalize_size(get_chunk_header_size() + requested_size);
  int idx = get_bin_index(requested_size);

  ChunkHeader *victim = nullptr;

  // Search for an exact-fit free chunk from the corresponding bin.
  if (idx < nr_bins && (victim = _bins[idx])) {
    victim->set_allocated(true);
    bin_del_head(victim);
    goto out;
  }

  // Search larger free chunks from the unsorted bin.
  for (ChunkHeader *chunk = _unsorted_bin; chunk; chunk = chunk->next) {
    if (chunk->get_size() >= requested_size) {
      victim = split_from_chunk(chunk, requested_size);
      goto out;
    }
  }

  // Search larger free chunks, and attempt to split an exact-fit chunk
  // from a larger free chunk.
  for (; idx < nr_bins; idx++) {
    if (_bins[idx]) {
      victim = split_from_chunk(_bins[idx], requested_size);
      goto out;
    }
  }

  if (is_top_chunk_used_up()) {
    if (!request_new_page_frame()) {
      return nullptr;
    }
  } else if (!is_top_chunk_large_enough(requested_size)) {
    // The top chunk hasn't been used up yet, but it is not large enough
    // to satisfy current request, so we could put the remainder into
    // the corresponding bin (if possible), and finally request a new
    // page frame which we can split from.
    ChunkHeader *chunk = split_from_top_chunk(get_top_chunk_size());

    if (is_chunk_size_usable(chunk->get_size())) {
      chunk->set_allocated(false);
      bin_add_head(chunk);
    } else {
      discard(chunk);
    }

    if (!request_new_page_frame()) {
      return nullptr;
    }
  }

  victim = split_from_top_chunk(requested_size);

out:
  if (Page::is_aligned(victim + 1)) [[unlikely]] {
    Kernel::panic(
        "fatal error: _top_chunk = 0x%x, _page_frame_allocatable_begin = 0x%x, req_size = "
        "0x%x, victim = 0x%x\n",
        _top_chunk, _page_frame_allocatable_begin, requested_size, victim);
  }

  return victim + 1;  // +1 to skip the header
}

void SlobAllocator::deallocate(void *p) {
  if (!p) [[unlikely]] {
    return;
  }

  // The final chunk pointer and size (after being merged)
  ChunkHeader *chunk = ChunkHeader::from_addr(p) - 1;  // -1 is for the header
  size_t chunk_size = chunk->get_size();

  // Mark current chunk as unallocated.
  chunk->set_allocated(false);

  // Maybe merge this chunk with its previous one.
  // The first chunk of a page frame has no previous chunk.
  if (!Page::is_aligned(chunk->addr())) {
    ChunkHeader *prev_chunk = chunk->get_prev_chunk();

    if (!prev_chunk->is_allocated()) {
      bin_del_entry(prev_chunk);
      chunk_size += prev_chunk->get_size();
      chunk = prev_chunk;
    }
  }

  // Maybe merge this chunk with its next one.
  const size_t next_chunk_addr = chunk->addr() + chunk_size;

  if (next_chunk_addr == reinterpret_cast<size_t>(_top_chunk)) {
    // The next one is the top chunk, merge `chunk` into the top chunk.
    _top_chunk = chunk;
    _top_chunk_prev_chunk_size = chunk->get_prev_chunk_size();
    return;
  }

  if (!Page::is_aligned(next_chunk_addr)) {
    ChunkHeader *next_chunk = ChunkHeader::from_addr(next_chunk_addr);

    if (!next_chunk->is_allocated()) {
      bin_del_entry(next_chunk);
      chunk_size += next_chunk->get_size();
    }
  }

  // Put the (maybe merged) chunk to the bin.
  chunk->next = nullptr;
  chunk->index = get_bin_index(chunk_size);
  set_next_chunk_prev_chunk_size(chunk);
  bin_add_head(chunk);
}

String SlobAllocator::to_string() const {
  String ret =
      "slobinfo\n"
      "--------\n";
  char linebuf[64] = {};

  ChunkHeader *ptr = nullptr;

  for (int i = 0; i < nr_bins; i++) {
    sprintf(linebuf, "_bins[%d] (%d): ", i, smallest_chunk_size + chunk_size_gap * i);
    ret += linebuf;

    ptr = _bins[i];
    while (ptr) {
      sprintf(linebuf, "[%d 0x%x] -> ", ptr->get_size(), ptr);
      ret += linebuf;
      ptr = ptr->next;
    }
    sprintf(linebuf, "(null)\n");
    ret += linebuf;
  }

  sprintf(linebuf, "_unsorted_bin: ");
  ret += linebuf;

  ptr = _unsorted_bin;
  while (ptr) {
    sprintf(linebuf, "[%d 0x%x] -> ", ptr->get_size(), ptr);
    ret += linebuf;
    ptr = ptr->next;
  }
  sprintf(linebuf, "(null)\n\n");
  ret += linebuf;

  sprintf(linebuf, "_page_frame_allocatable_begin = 0x%x\n", _page_frame_allocatable_begin);
  ret += linebuf;
  sprintf(linebuf, "_top_chunk                    = 0x%x\n", _top_chunk);
  ret += linebuf;
  sprintf(linebuf, "_page_frame_allocatable_end   = 0x%x\n", _page_frame_allocatable_end);
  ret += linebuf;
  return ret;
}

void SlobAllocator::dump() const {
  printf("--- dumping slob bins ---\n");
  printf("%s", to_string().c_str());
  printf("--- end dumping slob bins ---\n");
}

SlobAllocator::ChunkHeader *SlobAllocator::split_from_top_chunk(size_t requested_size) {
  if (requested_size > get_top_chunk_size()) [[unlikely]] {
    Kernel::panic("kernel heap corrupted (unable to split %d bytes from the top chunk)",
                  requested_size);
  }

  ChunkHeader *chunk = ChunkHeader::from_addr(_top_chunk);
  chunk->next = nullptr;
  chunk->index = get_bin_index(requested_size);
  chunk->prev_chunk_size = _top_chunk_prev_chunk_size;
  chunk->set_allocated(true);

  _top_chunk = reinterpret_cast<uint8_t *>(_top_chunk) + requested_size;
  _top_chunk_prev_chunk_size = requested_size;

  return chunk;
}

SlobAllocator::ChunkHeader *SlobAllocator::split_from_chunk(SlobAllocator::ChunkHeader *chunk,
                                                            const size_t requested_size) {
  if (!chunk) [[unlikely]] {
    Kernel::panic("kernel heap corrupted (chunk == nullptr)\n");
  }

  if (!is_chunk_size_usable(requested_size)) [[unlikely]] {
    Kernel::panic("kernel heap corrupted (requested_size must contain header size)\n");
  }

  if (chunk->index < 0) [[unlikely]] {
    Kernel::panic("kernel heap corrupted (invalid chunk->index: %d)\n", chunk->index);
  }

  if (requested_size > chunk->get_size()) [[unlikely]] {
    Kernel::panic(
        "kernel heap corrupted"
        "(unable to split %d bytes from a %d byte chunk)",
        requested_size, chunk->get_size());
  }

  const size_t remainder_size = chunk->get_size() - requested_size;

  bin_del_entry(chunk);
  chunk->next = nullptr;
  chunk->index = get_bin_index(requested_size);
  chunk->set_allocated(true);

  // Exact fit: there's no remainder, and the next chunk's prev_chunk_size
  // is still correct.
  if (!remainder_size) {
    return chunk;
  }

  ChunkHeader *remainder = ChunkHeader::from_addr(chunk->addr() + requested_size);
  remainder->next = nullptr;
  remainder->index = get_bin_index(remainder_size);
  remainder->prev_chunk_size = requested_size;
  set_next_chunk_prev_chunk_size(remainder);

  if (is_chunk_size_usable(remainder_size)) [[likely]] {
    remainder->set_allocated(false);
    bin_add_head(remainder);
  } else {
    discard(remainder);
  }

  return chunk;
}

void SlobAllocator::set_next_chunk_prev_chunk_size(const ChunkHeader *chunk) {
  const size_t next_chunk_addr = chunk->addr() + chunk->get_size();

  // The next chunk belongs to another page frame, don't touch it!
  if (Page::is_aligned(next_chunk_addr)) {
    return;
  }

  // The top chunk has no header, its prev_chunk_size is kept separately.
  if (next_chunk_addr == reinterpret_cast<size_t>(_top_chunk)) {
    _top_chunk_prev_chunk_size = chunk->get_size();
    return;
  }

  ChunkHeader::from_addr(next_chunk_addr)->set_prev_chunk_size(chunk->get_size());
}

bool SlobAllocator::request_new_page_frame() {
  void *page_frame = _buddy_allocator->allocate_one_page_frame();

  if (!page_frame) [[unlikely]] {
    return false;
  }

  _page_frame_allocatable_begin = page_frame;
  _top_chunk = _page_frame_allocatable_begin;
  _page_frame_allocatable_end = reinterpret_cast<char *>(_top_chunk) + PAGE_SIZE;
  _top_chunk_prev_chunk_size = 0;
  return true;
}

void SlobAllocator::bin_del_head(SlobAllocator::ChunkHeader *chunk) {
  if (!chunk) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: bin_del_head(nullptr)\n");
  }

  // If `chunk` doesn't fit in regular bins,
  // we should use bin_del_entry() to remove it from the unsorted bin.
  if (chunk->index >= nr_bins) {
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

void SlobAllocator::bin_add_head(SlobAllocator::ChunkHeader *chunk) {
  if (!chunk) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: bin_add_head(nullptr)\n");
  }

  ChunkHeader **bin_first_chunk = bin_get_head(chunk);

  if (*bin_first_chunk == chunk) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: repeated bin_add_head()\n");
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

void SlobAllocator::bin_del_entry(SlobAllocator::ChunkHeader *chunk) {
  if (!chunk) [[unlikely]] {
    Kernel::panic("kernel heap corrupted: bin_del_entry(nullptr)\n");
  }

  ChunkHeader **bin_first_chunk = bin_get_head(chunk);

  if (*bin_first_chunk == chunk) {
    *bin_first_chunk = (*bin_first_chunk)->next;
    chunk->next = nullptr;
    return;
  }

  ChunkHeader *prev = nullptr;
  ChunkHeader *ptr = *bin_first_chunk;

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

}  // namespace valkyrie::kernel
