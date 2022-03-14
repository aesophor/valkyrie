// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_USERSPACE_ACCESS_H_
#define VALKYRIE_USERSPACE_ACCESS_H_

// Marks a pointer as a userspace pointer, indicating that we shouldn't
// trust it or even simply dereference it.
#define __user

#endif  // VALKYRIE_USERSPACE_ACCESS_H_
