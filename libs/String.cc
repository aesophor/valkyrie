// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <String.h>

namespace valkyrie::kernel {

void* memcpy(void* dest, const void* src, size_t n) {

}

void* memset(void* dest, uint8_t val, size_t n) {
  for (size_t i = 0; i < n; i++) {
    reinterpret_cast<uint8_t*>(dest)[i] = val;
  }
  return dest;
}


size_t strlen(const char* s) {
  size_t len;
  for (len = 0; *s; s++) ++len;
  return len;
}

int strcmp(const char* s1, const char* s2) {
  while (*s1 || *s2) {
    if (*s1 == *s2) {
      ++s1;
      ++s2;
    } else {
      return *s1 - *s2;
    }
  }
  return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {

}

char* strcpy(char* dest, const char* src) {

}

char* strncpy(char* dest, const char* src, size_t n) {

}

char* strcat(char* dest, const char* src) {

}

char* strstr(const char* haystack, const char* needle) {

}

}  // namespace valkyrie::kernel
