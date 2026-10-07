// Host stubs for the kernel bits that mm/SlobAllocator.cc depends on.
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#define PAGE_SIZE 4096
#define MAKE_NONCOPYABLE(C) C(const C &) = delete; C &operator=(const C &) = delete
#define MAKE_NONMOVABLE(C) C(C &&) = delete; C &operator=(C &&) = delete
namespace valkyrie::kernel {
using ::size_t; using ::int32_t; using ::uint8_t;
template <typename T> const T &max(const T &a, const T &b) { return a < b ? b : a; }
inline size_t round_up_to_multiple_of_n(size_t x, const size_t n) { size_t r = n; while (r < x) r += n; return r; }
struct String {
  std::string s;
  String(const char *c = "") : s(c) {}
  String &operator+=(const char *c) { s += c; return *this; }
  const char *c_str() const { return s.c_str(); }
};
template <typename... A> void printf(const char *f, A... a) { std::printf(f, a...); }
template <typename... A> void sprintf(char *b, const char *f, A... a) { std::sprintf(b, f, a...); }
template <typename... A> void printk(const char *f, A... a) { std::printf(f, a...); }
struct Kernel {
  template <typename... A> [[noreturn]] static void panic(const char *f, A... a) {
    std::fprintf(stderr, "PANIC: "); std::fprintf(stderr, f, a...); std::abort();
  }
};
struct Page {
  template <typename T> static bool is_aligned(T a) { return (size_t)a % PAGE_SIZE == 0; }
};
class BuddyAllocator {
 public:
  void *allocate_one_page_frame();
};
}  // namespace valkyrie::kernel
