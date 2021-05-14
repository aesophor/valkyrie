// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLOB_ALLOCATOR_H_
#define VALKYRIE_SLOB_ALLOCATOR_H_

#include <Types.h>

#define CHUNK_SMALLEST_SIZE static_cast<size_t>(0x10)
#define CHUNK_LARGEST_SIZE  static_cast<size_t>(0x80)
#define CHUNK_SIZE_GAP      0x10

#define NR_BINS                                                        \
  static_cast<int>(                                                    \
      (CHUNK_LARGEST_SIZE - CHUNK_SMALLEST_SIZE) / CHUNK_SIZE_GAP + 1)

namespace valkyrie::kernel {

// Forward Declaration
class BuddyAllocator;

class SlobAllocator {
 public:
  explicit SlobAllocator(BuddyAllocator* page_frame_allocator);

  ~SlobAllocator() = default;
  SlobAllocator(const SlobAllocator&) = delete;
  SlobAllocator(SlobAllocator&&) = delete;
  SlobAllocator& operator =(const SlobAllocator&) = delete;
  SlobAllocator& operator =(SlobAllocator&&) = delete;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_slob_info() const;

  static size_t get_chunk_header_size();

 private:
  struct Slob final {
    Slob* next;  // only reliable if current chunk is free!
    int32_t index;
    int32_t prev_chunk_size;

    size_t get_chunk_size() const;
    int32_t get_prev_chunk_size() const;
    void set_prev_chunk_size(const int32_t size);

    bool is_allocated() const;
    void set_allocated(bool allocated);
  };

  void request_new_page_frame();
  bool is_first_chunk_in_page_frame(const Slob* chunk) const;

  Slob* split_from_top_chunk(size_t requested_size);
  bool  is_top_chunk_used_up() const;
  bool  is_top_chunk_large_enough(const size_t requested_size) const;
  size_t get_top_chunk_size() const;

  Slob* split_chunk(Slob* chunk, const size_t target_size);

  void bin_del_head(Slob* chunk);
  void bin_add_head(Slob* chunk);
  void bin_del_entry(Slob* chunk);

  int get_bin_index(size_t size);
  size_t normalize_size(size_t size);


  BuddyAllocator* _buddy_allocator;
  void* _page_frame_allocatable_begin;
  void* _top_chunk;
  void* _page_frame_allocatable_end;
  int32_t _top_chunk_prev_chunk_size;

  Slob* _bins[NR_BINS];
  Slob* _unsorted_bin;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLOB_ALLOCATOR_H_
