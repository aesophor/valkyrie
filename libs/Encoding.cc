// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include <libs/Encoding.h>

namespace valkyrie::kernel {

// Convert UTF8-coded characters into Unicode2-coded. 
ucs2_char_t *utf2ucs (ucs2_char_t *dst, const utf8_char_t *src) {
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int next = 0;

  while (src[i] != 0) {
    ucs2_char_t temp = 0;

    if (src[i] & 0x80 && src[i+1] & 0x80) {
      next = 3;
      temp |= ((src[i] & 0xF) << 12);
      temp |= ((src[i+1] & 0x3F) << 6);
      temp |= ((src[i+2] & 0x3F) << 0);
    } else if(src[i] & 0x80 && src[i+1] & 0xC0) {
      next = 2;
      temp |= (src[i] & 0x1F) << 6;
      temp |= (src[i+1] & 0x3F) << 0;
    } else {
      next = 1;
      temp = src[i];
    }

    dst[j++] = temp;
    i += next;
  }

  dst[j] = 0;
  return dst;
};


// Convert Unicode2-coded characters into UTF8-coded. 
utf8_char_t *ucs2utf (utf8_char_t *dst, const ucs2_char_t *src) {
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int next = 0;

  while (src[i] != 0) {
    if (src[i] < 0x80) { 
      next = 1;
      dst[j] = 0;
      dst[j] = src[i];
    } else if(src[i] < 0x800) {
      next = 2;
      dst[j] = 0;
      dst[j+1] = 0;
      dst[j+1] = (utf8_char_t) ((src[i] & 0x3F) | 0x80);
      dst[j] = (utf8_char_t) (((src[i] & 0x3F) & 0x1F) | 0xC0);
    } else {
      next = 3;
      dst[j] = 0;
      dst[j+1] = 0;
      dst[j+2] = 0;
      dst[j] |= ((((utf8_char_t) (src[i] >> 12)) & 0xF) | 0xE0);
      dst[j+1] |= (((utf8_char_t) (src[i] >> 6) & 0x3F) | 0x80);
      dst[j+2] |= (((utf8_char_t) (src[i] >> 0) & 0x3F) | 0x80);
    }

    j += next;
    i++;
  }

  dst[j] = 0;
  return dst;
}

}  // namespace valkyrie::kernel
