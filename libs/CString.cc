// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <libs/CString.h>

extern "C" {

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

int memcmp(const void* ptr1, const void* ptr2, size_t num) {
  const uint8_t* p1 = reinterpret_cast<const uint8_t*>(ptr1);
  const uint8_t* p2 = reinterpret_cast<const uint8_t*>(ptr2);

  while (num-- > 0) {
    if (*p1++ != *p2++) {
      return p1[-1] < p2[-1] ? -1 : 1;
    }
  }
  return 0;
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


int atoi(const char* str, const int base) {
  static auto is_digit = [](const char c) -> bool {
    return c >= '0' && c <= '9';
  };

  static auto is_alphanumeric = [](const char c) -> bool {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
  };

  static auto to_decimal = [](const char c) -> int {
    int ret = 0;
    if (is_digit(c)) {
      ret = c - '0';
    } else if (c >= 'A' && c <= 'F') {
      ret = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      ret = c - 'a' + 10;
    }
    return ret;
  };

  // TODO: assert(base == 10 || base == 16)
  bool is_negative = false;
  size_t len = 0;
  size_t pos = 0;
  int result = 0;
  int ptr = 0;
  auto is_valid = (base == 10) ? is_digit : is_alphanumeric;

  // Remove preceding whitespaces
  while (str[pos] == ' ') {
    if (str[pos] == 0) {
      return 0;
    }
    ++pos;
  }

  str = str + pos;
  len = strlen(str);

  if (str[0] != '+' && str[0] != '-' && !is_valid(str[0])) {
    return 0;
  }

  switch (str[0]) {
    case '-':
      is_negative = true;
      ptr = 1;
      break;

    case '+':
      ptr = 1;
      break;

    default:
      break;
  }

  for (; ptr < (int) len && is_valid(str[ptr]); ptr++) {
    if (result > INT_MAX / base) {
      return (is_negative) ? INT_MIN : INT_MAX;
    }
    result *= base;

    int next_digit = to_decimal(str[ptr]);
    if (result > INT_MAX - next_digit) {
      return (is_negative) ? INT_MIN : INT_MAX;
    }
    result += next_digit;
  }

  return (is_negative) ? -result : result;
}

}
