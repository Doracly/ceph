// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:nil -*-
// vim: ts=8 sw=2 sts=2 expandtab

/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include "common/mutex_debug.h"
#include "common/perf_counters.h"
#include "common/ceph_context.h"
#include "common/config.h"

namespace ceph {
namespace mutex_debug_detail {
enum {
  l_mutex_first = 999082,
  l_mutex_wait,
  l_mutex_last
};

mutex_debugging_base::mutex_debugging_base(std::string group, bool ld, bool bt)
  : group(std::move(group)),
    lockdep(ld),
    backtrace(bt)
{
  if (_enable_lockdep()) {
    _register();
  }
}

mutex_debugging_base::~mutex_debugging_base() {
  ceph_assert(nlock == 0);
  // unconditionally, lockdep may have been disabled since; -1 is ignored
  lockdep_unregister(id.load(std::memory_order_relaxed));
}

// Registers on first use.  Each registration takes a reference that the dtor
// drops only once, so a thread losing the race returns its own.
int mutex_debugging_base::_lockdep_id() {
  int cur = id.load(std::memory_order_relaxed);
  if (cur >= 0) {
    return cur;
  }
  const int fresh = lockdep_register(group.c_str());
  if (fresh < 0) {
    return fresh; // lockdep disabled
  }
  if (id.compare_exchange_strong(cur, fresh, std::memory_order_relaxed,
                                 std::memory_order_relaxed)) {
    return fresh;
  }
  lockdep_unregister(fresh);
  return cur; // the winner's id
}

void mutex_debugging_base::_register() {
  id.store(lockdep_register(group.c_str()), std::memory_order_relaxed);
}
void mutex_debugging_base::_will_lock(bool recursive) { // about to lock
  lockdep_will_lock(group.c_str(), _lockdep_id(), backtrace, recursive);
}
void mutex_debugging_base::_locked() {    // just locked
  lockdep_locked(group.c_str(), _lockdep_id(), backtrace);
}
void mutex_debugging_base::_will_unlock() {  // about to unlock
  lockdep_will_unlock(group.c_str(), _lockdep_id());
}

} // namespace mutex_debug_detail
} // namespace ceph
