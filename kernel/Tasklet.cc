// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
//
// A tasklet is a special form of deferred work that runs in interrupt context,
// just like softirqs. The main difference between sofirqs and tasklets is that
// tasklets can be allocated dynamically and thus they can be used by device drivers.
//
// Reference:
// [1] https://grasslab.github.io/NYCU_Operating_System_Capstone/labs/lab4.html
// [2] https://linux-kernel-labs.github.io/refs/heads/master/labs/deferred_work.html#tasklets

#include <kernel/Tasklet.h>

#include <Utility.h>

namespace valkyrie::kernel {

}  // namespace valkyrie::kernel
