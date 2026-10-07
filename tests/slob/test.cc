// Host-side stress test for valkyrie's SlobAllocator.
#include <algorithm>
#include <cstring>
#include <map>
#include <random>
#include <set>
#include <vector>
#include <mm/SlobAllocator.h>

using namespace valkyrie::kernel;

static constexpr int NR_POOL_PAGES = 2048;
alignas(4096) static uint8_t pool[NR_POOL_PAGES * PAGE_SIZE];
static int next_page = 0;  // slob gets even pages; odd pages are foreign (canary-filled)
static int slob_pages = 0;
static uint8_t *slob_page_list[NR_POOL_PAGES];
static constexpr uint8_t CANARY = 0xAB;

void *BuddyAllocator::allocate_one_page_frame() {
  if (next_page >= NR_POOL_PAGES) return nullptr;
  uint8_t *p = pool + next_page * PAGE_SIZE;
  next_page += 2;
  slob_page_list[slob_pages++] = p;
  return p;
}

static int failures = 0;
#define CHECK(cond, ...)                 \
  do {                                   \
    if (!(cond)) {                       \
      std::printf("FAIL: " __VA_ARGS__); \
      std::printf("\n");                 \
      if (++failures > 5) std::exit(1);  \
    }                                    \
  } while (0)

static void check_foreign_pages(const char *when) {
  for (int i = 1; i < NR_POOL_PAGES; i += 2) {
    for (int j = 0; j < PAGE_SIZE; j++) {
      if (pool[i * PAGE_SIZE + j] != CANARY) {
        CHECK(false, "write into foreign page %d offset %d (%s)", i, j, when);
        std::memset(pool + i * PAGE_SIZE, CANARY, PAGE_SIZE);
        return;
      }
    }
  }
}


// Walks every slob page and checks chunk headers. Needs private access.
static void check_heap(SlobAllocator &s, const char *when) {
  using CH = SlobAllocator::ChunkHeader;
  std::set<CH *> binned;
  for (int i = 0; i < SlobAllocator::nr_bins; i++)
    for (CH *c = s._bins[i]; c; c = c->next) CHECK(binned.insert(c).second, "chunk %p in bins twice (%s)", c, when);
  for (CH *c = s._unsorted_bin; c; c = c->next) CHECK(binned.insert(c).second, "chunk %p in bins twice (%s)", c, when);

  for (int i = 0; i < slob_pages; i++) {
    size_t addr = (size_t)slob_page_list[i], end = addr + PAGE_SIZE;
    size_t prev_size = 0;
    while (addr < end && addr != (size_t)s._top_chunk) {
      CH *c = CH::from_addr(addr);
      size_t size = c->get_size();
      if (!(size >= 16 && addr + size <= end && c->get_prev_chunk_size() == (int32_t)prev_size)) {
        CHECK(false, "bad chunk %p size %zu prev %d (expected %zu) (%s)", c, size, c->get_prev_chunk_size(), prev_size, when);
        return;
      }
      if (!c->is_allocated()) CHECK(binned.count(c), "free chunk %p (size %zu) is in no bin: leaked (%s)", c, size, when);
      binned.erase(c);
      prev_size = size;
      addr += size;
    }
    if (addr == (size_t)s._top_chunk && addr != end)
      CHECK(s._top_chunk_prev_chunk_size == (int32_t)prev_size, "top chunk prev size %d, expected %zu (%s)", s._top_chunk_prev_chunk_size, prev_size, when);
  }
  CHECK(binned.empty(), "%zu binned chunks are not real chunks (%s)", binned.size(), when);
}

struct Alloc {
  size_t size;
  uint8_t tag;
};

static bool verify(uint8_t *p, const Alloc &a) {
  for (size_t i = 0; i < a.size; i++)
    if (p[i] != a.tag) return false;
  return true;
}

int main(int argc, char **argv) {
  unsigned seed = argc > 1 ? std::atoi(argv[1]) : 1;
  std::memset(pool, CANARY, sizeof(pool));
  static SlobAllocator slob(new BuddyAllocator);
  std::mt19937 rng(seed);
  std::map<uint8_t *, Alloc> live;
  uint8_t tag = 1;

  auto rand_size = [&]() -> size_t {
    switch (rng() % 4) {
      case 0: return 1 + rng() % 0x70;                        // small bins
      case 1: return 0x70 + rng() % 0x100;                    // around the unsorted threshold
      case 2: return 16 * (1 + rng() % 40);                   // multiples of 16 -> exact fits
      default: return 1 + rng() % (PAGE_SIZE - 16 - 1);      // up to kmalloc's slob limit
    }
  };

  // Phase 1: random alloc/free.
  for (int op = 0; op < 100000; op++) {
    if (live.empty() || rng() % 100 < (live.size() < 400 ? 60 : 40)) {
      size_t size = rand_size();
      auto *p = static_cast<uint8_t *>(slob.allocate(size));
      if (!p) break;  // pool exhausted
      CHECK((size_t)p % 16 == 0, "misaligned pointer %p", p);
      auto nb = live.lower_bound(p);
      if (nb != live.end())
        CHECK(p + size <= nb->first, "overlap: new %p+%zu vs live %p (op %d)", p, size, nb->first, op);
      if (nb != live.begin()) {
        --nb;
        CHECK(nb->first + nb->second.size <= p, "overlap: live %p+%zu vs new %p (op %d)", nb->first,
              nb->second.size, p, op);
      }
      std::memset(p, tag, size);
      live[p] = {size, tag};
      tag = tag == 0xAA ? 1 : tag + 1;
    } else {
      auto it = live.begin();
      std::advance(it, rng() % live.size());
      CHECK(verify(it->first, it->second), "allocation %p+%zu clobbered before free (op %d)", it->first,
            it->second.size, op);
      slob.deallocate(it->first);
      live.erase(it);
    }

    if (op % 2000 == 0) {
      check_foreign_pages("phase 1");
      check_heap(slob, "phase 1");
      for (auto &[q, a] : live) {
        if (!verify(q, a)) {
          CHECK(false, "live allocation %p+%zu clobbered (op %d)", q, a.size, op);
          break;
        }
      }
    }
  }
  for (auto &[q, a] : live) slob.deallocate(q);
  live.clear();
  check_foreign_pages("after phase 1");
  check_heap(slob, "after phase 1");

  // Phase 2: reuse. Repeated identical alloc-all/free-all rounds
  // must not keep requesting new pages.
  int pages_before = slob_pages;
  for (int round = 0; round < 50; round++) {
    std::vector<void *> ps;
    for (int i = 0; i < 64; i++) ps.push_back(slob.allocate(0x30));
    for (void *p : ps) CHECK(p, "allocation failed in reuse round %d", round);
    std::shuffle(ps.begin(), ps.end(), rng);
    for (void *p : ps) slob.deallocate(p);
  }
  int grew = slob_pages - pages_before;
  CHECK(grew <= 2, "freed memory not reused: %d new pages over 50 identical rounds", grew);
  check_foreign_pages("phase 2");
  check_heap(slob, "phase 2");

  std::printf("seed %u: %s (%d slob pages used)\n", seed, failures ? "FAILED" : "ok", slob_pages);
  return failures ? 1 : 0;
}
