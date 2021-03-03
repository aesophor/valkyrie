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
    if(dest == NULL)
        return NULL;

    char *ret = dest;

    while(*src != '\0'){
        *dest++ = *src++;
    }
    *dest = '\0';

    return ret;
}

char* strncpy(char* dest, const char* src, size_t n) {

}

char* strcat(char* dest, const char* src) {

}

char* strstr(const char* haystack, const char* needle) {

}
