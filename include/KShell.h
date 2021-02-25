// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_KSHELL_H_
#define VALKYRIE_KSHELL_H_

#include <Types.h>

#define KSHELL_BUF_SIZE 256

namespace valkyrie::kernel {

class KShell {
 public:
  KShell();
  ~KShell() = default;

  void run();

 private:
  char _buf[KSHELL_BUF_SIZE];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_KSHELL_H_
