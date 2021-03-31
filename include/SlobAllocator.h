// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SLOB_ALLOCATOR_H_
#define VALKYRIE_SLOB_ALLOCATOR_H_

#include <PageFrameAllocator.h>

#define CHUNK_SMALLEST_SIZE 0x20
#define CHUNK_LARGEST_SIZE  0x80
#define CHUNK_SIZE_GAP      0x10

#define NUM_OF_BINS \
  ((CHUNK_LARGEST_SIZE - CHUNK_SMALLEST_SIZE) / CHUNK_SIZE_GAP + 1)

namespace valkyrie::kernel {

class SlobAllocator {
 public:
  explicit SlobAllocator(PageFrameAllocator* page_frame_allocator);
  ~SlobAllocator() = default;

  void* allocate(size_t requested_size);
  void  deallocate(void* p);
  void  dump_slob_info() const;

 private:
  struct Slob final {
    Slob* next;
    int32_t index;
    int32_t prev_chunk_size;

    int32_t get_prev_chunk_size() const;
    bool is_allocated() const;
    void set_allocated(bool allocated);
  };

  void request_new_page_frame();

  Slob* split_from_top_chunk(size_t requested_size);
  bool  is_top_chunk_used_up() const;
  bool  is_top_chunk_large_enough(const size_t requested_size) const;
  size_t get_top_chunk_size() const;

  Slob* split_chunk(Slob* chunk, const size_t target_size);

  void bin_del_head(Slob* chunk);
  void bin_add_head(Slob* chunk);
  void bin_del_entry(Slob* chunk);

  void unsorted_bin_add_head(Slob* chunk);
  void unsorted_bin_del_entry(Slob* chunk);

  int get_bin_index(size_t size);
  size_t get_chunk_size(const int index) const;

  size_t sanitize_size(size_t size);
  size_t round_up_to_multiple_of_16(size_t x);


  PageFrameAllocator* _page_frame_allocator;
  void* _top_chunk;
  void* _top_chunk_end;

  Slob* _bins[NUM_OF_BINS];
  Slob* _unsorted_bin;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SLOB_ALLOCATOR_H_
