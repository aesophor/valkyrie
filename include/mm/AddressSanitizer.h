// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_ADDRESS_SANITIZER_H_
#define VALKYRIE_ADDRESS_SANITIZER_H_

namespace valkyrie::kernel {

class AddressSanitizer {
 public:
  AddressSanitizer();
  ~AddressSanitizer() = default;

  bool mark_free_chk(void* p);
  void mark_allocated(void *p);
  void show();

 private:
  void* _allocated_pointers[1000];
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_ADDRESS_SANITIZER_H_
