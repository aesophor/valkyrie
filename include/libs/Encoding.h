// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// https://github.com/smoothwind/ucs2-utf8/blob/master/ucs2-utf8-release.c

#ifndef VALKYRIE_ENCODING_H_
#define VALKYRIE_ENCODING_H_

namespace valkyrie::kernel {

using ucs2_char_t = unsigned short;
using utf8_char_t = unsigned char;

// Convert UTF8-coded characters into Unicode2-coded. 
ucs2_char_t* utf2ucs(ucs2_char_t* dst, const utf8_char_t* src);

// Convert Unicode2-coded characters into UTF8-coded. 
utf8_char_t* ucs2utf(utf8_char_t* dst, const ucs2_char_t* src);

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ENCODING_H_
