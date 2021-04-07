// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_C_STRING_H_
#define VALKYRIE_C_STRING_H_

#include <Types.h>

extern "C" {

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, uint8_t val, size_t n);
int memcmp(const void* ptr1, const void* ptr2, size_t num);

size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strstr(const char* haystack, const char* needle);

// Convert the given C string `str` to a signed integer.
// `base` can only be either: 10 or 16.
// When `base` == 16, it should NOT start with "0x"
int atoi(const char* str, const int base = 10);

}

#endif  // VALKYRIE_C_STRING_H_
