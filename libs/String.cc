// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <String.h>

void* memcpy(void* dest, const void* src, size_t n) {
  const uint8_t* src_p = reinterpret_cast<const uint8_t*>(src);
  uint8_t* dest_p = reinterpret_cast<uint8_t*>(dest);

  for (; n > 0; n--) {
    *dest_p++ = *src_p++;
  }
  return dest;
}

void* memset(void* dest, uint8_t val, size_t n) {
  uint8_t* dest_p = reinterpret_cast<uint8_t*>(dest);

  for (; n > 0; n--) {
    *dest_p++ = val;
  }
  return dest;
}


size_t strlen(const char* s) {
  size_t len = 0;

  for (; *s; s++) {
    ++len;
  }
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
  while ((*s1 || *s2) && n--) {
    if (*s1 == *s2) {
      ++s1;
      ++s2;
    } else {
      return *s1 - *s2;
    }
  }
  return 0;
}

char* strcpy(char* dest, const char* src) {
  char* ret = dest;
  while (*src) {
    *dest++ = *src++;
  }
  *dest = '\0';
  return ret;
}

char* strncpy(char* dest, const char* src, size_t n) {
  char* ret = dest;
  while (*src && n--) {
    *dest++ = *src++;
  }
  *dest = '\0';
  return ret;
}

char* strcat(char* dest, const char* src) {
  char* cur = dest + strlen(dest);
  while (*src) {
    *cur++ = *src++;
  }
  *cur = '\0';
  return dest;
}

char* strstr(const char* haystack, const char* needle){
  int haystack_size = strlen(haystack);
  int needle_size = strlen(needle);

  if (!needle_size) {
    return nullptr;
  }

  for (int i = 0; i <= haystack_size - needle_size; i++) {
    bool matched = true;

    for (int j = 0; j < needle_size; j++) {
      if (haystack[i + j] != needle[j]) {
        matched = false;
        break;
      }
    }

    if (matched) {
      return const_cast<char*>(haystack + i);
    }
  }
  return nullptr;
}
