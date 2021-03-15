// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_SHELL_H_
#define VALKYRIE_SHELL_H_

#include <Types.h>

#define SHELL_BUF_SIZE 256

namespace valkyrie::kernel {

class Shell {
 public:
  Shell();
  ~Shell() = default;

  void run();

 private:
  char _buf[SHELL_BUF_SIZE];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_SHELL_H_
