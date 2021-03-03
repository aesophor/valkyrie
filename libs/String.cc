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
   if(n == 0)
       return 0;

    while(n--){
        if(*s1 != *s2)
            return *s1 - *s2;
        else{
            s1 ++;
            s2 ++;
        }
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    if(dest == nullptr)
        return nullptr;

    char *ret = dest;

    while(*src != '\0'){
        *dest++ = *src++;
    }
    *dest = '\0';

    return ret;
}

char* strncpy(char* dest, const char* src, size_t n) {
    if(dest == nullptr)
        return nullptr;

    char *ret = dest;

    while(*src != '\0' && n --){
        *dest++ = *src++;
    }

    *dest = '\0';

    return ret;

}

char* strcat(char* dest, const char* src) {
    char *cur = dest + strlen(dest);

    while(*src != '\0'){
        *cur++ = *src++;
    }

    *cur = '\0';

    return dest;
}

int compare(const char* x, const char* y){
    while(*x && *y){
        if(*x != *y)
            return 0;
        x++;
        y++;
    }
    return (*y == '\0');
}
char* strstr(const char* haystack, const char* needle){
    while(*haystack != '\0'){
        if(*haystack == *needle && compare(haystack, needle))
            return ((char*)haystack);
        haystack ++;
    }
    return nullptr;
}
