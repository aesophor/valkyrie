// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLOB_ALLOCATOR_H_
#define VALKYRIE_SLOB_ALLOCATOR_H_

#include <Algorithm.h>
#include <Math.h>
#include <String.h>
#include <TypeTraits.h>

namespace valkyrie::kernel {

// Forward Declaration
class BuddyAllocator;

class SlobAllocator {
  MAKE_NONCOPYABLE(SlobAllocator);
  MAKE_NONMOVABLE(SlobAllocator);

 public:
  explicit SlobAllocator(BuddyAllocator* page_frame_allocator);
  ~SlobAllocator() = default;

  void* allocate(size_t requested_size);
  void deallocate(void* p);

  String to_string() const;
  void dump() const;

  static constexpr size_t get_chunk_header_size() {
    return sizeof(ChunkHeader);
  }

 private:
  static constexpr const size_t smallest_chunk_size = 0x20;
  static constexpr const size_t largest_chunk_size = 0x80;
  static constexpr const size_t chunk_size_gap = 0x10;

  static constexpr const int nr_bins
    = (largest_chunk_size - smallest_chunk_size) / chunk_size_gap + 1;

  struct ChunkHeader final {
    ChunkHeader* next;        // only reliable if current chunk is free!
    int32_t index;            // contains header size
    int32_t prev_chunk_size;  // contains header size

    template <typename T>
    static ChunkHeader* from_addr(T addr) {
      return reinterpret_cast<ChunkHeader*>(addr);
    }

    size_t addr() const {
      return reinterpret_cast<size_t>(this);
    }

    size_t get_size() const {
      return smallest_chunk_size + index * chunk_size_gap;
    }

    ChunkHeader* get_prev_chunk() const {
      return from_addr(addr() - get_prev_chunk_size());
    }

    ChunkHeader* get_next_chunk() const {
      return from_addr(addr() + get_size());
    }

    int32_t get_prev_chunk_size() const {
      return prev_chunk_size & ~1;
    }

    void set_prev_chunk_size(const int32_t size) {
      bool flag = is_allocated();
      prev_chunk_size = size;
      set_allocated(flag);
    }

    bool is_allocated() const {
      return prev_chunk_size & 1;
    }

    void set_allocated(bool allocated) {
      allocated ? prev_chunk_size |= 1 : prev_chunk_size &= ~1;
    }
  };

  bool is_chunk_size_usable(size_t chunk_size) const {
    return chunk_size >= smallest_chunk_size;
  }

  bool is_top_chunk_used_up() const {
    return get_top_chunk_size() == 0;
  }

  bool is_top_chunk_large_enough(const size_t requested_size) const {
    return get_top_chunk_size() >= requested_size;
  }

  size_t get_top_chunk_size() const {
    return reinterpret_cast<size_t>(_page_frame_allocatable_end) -
           reinterpret_cast<size_t>(_top_chunk);
  }


  ChunkHeader* split_from_top_chunk(size_t requested_size);
  ChunkHeader* split_from_chunk(ChunkHeader* chunk, const size_t requested_size);

  bool request_new_page_frame();

  void bin_del_head(ChunkHeader* chunk);
  void bin_add_head(ChunkHeader* chunk);
  void bin_del_entry(ChunkHeader* chunk);

  ChunkHeader** bin_get_head(ChunkHeader* chunk) {
    // If `chunk->index` >= nr_bins, then we need to look for `chunk`
    // from the unsorted bin instead of regular bins.
    return chunk->index >= nr_bins ? &_unsorted_bin : &_bins[chunk->index];
  }

  // XXX: Maybe this can be optimized later.
  void discard(ChunkHeader* chunk) {
    // Discard the chunk parmanently by setting it to allocated,
    // preventing it from being merged with adjacent free chunks.
    chunk->set_allocated(true);
  }

  int get_bin_index(size_t size) {
    return (size - smallest_chunk_size) / chunk_size_gap;
  }

  size_t normalize_size(size_t size) {
    return round_up_to_multiple_of_n(max(size, smallest_chunk_size), 16);
  }


  BuddyAllocator* _buddy_allocator;
  void* _page_frame_allocatable_begin;
  void* _top_chunk;
  void* _page_frame_allocatable_end;
  int32_t _top_chunk_prev_chunk_size;

  // Each bin in `_bins` stores chunks of the same size.
  ChunkHeader* _bins[nr_bins];

  // The unsorted bin stores chunks of different sizes that are
  // too large to fit within `_bins`.
  ChunkHeader* _unsorted_bin;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLOB_ALLOCATOR_H_
