// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_PATH_LEXER_H_
#define VALKYRIE_PATH_LEXER_H_

#include <String.h>

namespace valkyrie::kernel {

class PathLexer final {
 public:
  static String normalize(String path);

 private:
  static void deduplicate_delimiters(String& path);
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_PATH_LEXER_H_
