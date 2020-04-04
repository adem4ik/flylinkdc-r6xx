/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_SYNCHRONIZATION_RW_LOCK_WRAPPER_H_
#define RTC_BASE_SYNCHRONIZATION_RW_LOCK_WRAPPER_H_

#include "webrtc/rtc_base/thread_annotations.h"

#include "CFlyLockProfiler.h" // FlylinkDC++

// Note, Windows pre-Vista version of RW locks are not supported natively. For
// these OSs regular critical sections have been used to approximate RW lock
// functionality and will therefore have worse performance.

namespace webrtc {

class RTC_LOCKABLE RWLockWrapper 
{
 public:
  static RWLockWrapper* CreateRWLock();
  virtual ~RWLockWrapper() {}

  virtual void AcquireLockExclusive() RTC_EXCLUSIVE_LOCK_FUNCTION() = 0;
  virtual void ReleaseLockExclusive() RTC_UNLOCK_FUNCTION() = 0;

  virtual void AcquireLockShared() RTC_SHARED_LOCK_FUNCTION() = 0;
  virtual void ReleaseLockShared() RTC_UNLOCK_FUNCTION() = 0;
};

// RAII extensions of the RW lock. Prevents Acquire/Release missmatches and
// provides more compact locking syntax.
class RTC_SCOPED_LOCKABLE ReadLockScoped 
#ifdef FLYLINKDC_USE_PROFILER_CS
	: public CFlyLockProfiler
#endif
{
 public:
  explicit ReadLockScoped(RWLockWrapper& rw_lock
#ifdef FLYLINKDC_USE_PROFILER_CS
	  , const char* p_function = nullptr
	  , int p_line = 0
#endif
)
      RTC_SHARED_LOCK_FUNCTION(rw_lock)
      : rw_lock_(rw_lock) 
#ifdef FLYLINKDC_USE_PROFILER_CS
	  , CFlyLockProfiler(p_function, p_line)
#endif
   {
    rw_lock_.AcquireLockShared();
#ifdef FLYLINKDC_USE_PROFILER_CS
	log("D:\\ReadSectionLog-lock.txt", 0);
#endif
  }

  ~ReadLockScoped() RTC_UNLOCK_FUNCTION() { 
rw_lock_.ReleaseLockShared();
#ifdef FLYLINKDC_USE_PROFILER_CS
	log("D:\\ReadSectionLog-unlock.txt", 0);
#endif

 }

 private:
  RWLockWrapper& rw_lock_;
};

class RTC_SCOPED_LOCKABLE WriteLockScoped 
#ifdef FLYLINKDC_USE_PROFILER_CS
	: public CFlyLockProfiler
#endif
{
 public:
  explicit WriteLockScoped(RWLockWrapper& rw_lock
#ifdef FLYLINKDC_USE_PROFILER_CS
	  , const char* p_function = nullptr
	  , int p_line = 0
#endif
)
      RTC_EXCLUSIVE_LOCK_FUNCTION(rw_lock)
      : rw_lock_(rw_lock) 
#ifdef FLYLINKDC_USE_PROFILER_CS
	  , CFlyLockProfiler(p_function,p_line)
#endif
{
    rw_lock_.AcquireLockExclusive();
#ifdef FLYLINKDC_USE_PROFILER_CS
	log("D:\\WriteSectionLog-lock.txt", 0);
#endif
  }

  ~WriteLockScoped() RTC_UNLOCK_FUNCTION() { 
rw_lock_.ReleaseLockExclusive(); 
#ifdef FLYLINKDC_USE_PROFILER_CS
	log("D:\\WriteSectionLog-unlock.txt", 0);
#endif
}

 private:
  RWLockWrapper& rw_lock_;
};

#ifdef FLYLINKDC_USE_PROFILER_CS
#define CFlyReadLock(cs) webrtc::ReadLockScoped l_lock(cs,__FUNCTION__, __LINE__);
#define CFlyWriteLock(cs) webrtc::WriteLockScoped l_lock(cs,__FUNCTION__, __LINE__);
#else
#define CFlyReadLock(cs) webrtc::ReadLockScoped l_lock(cs);
#define CFlyWriteLock(cs) webrtc::WriteLockScoped l_lock(cs);
#endif

}  // namespace webrtc

#endif  // RTC_BASE_SYNCHRONIZATION_RW_LOCK_WRAPPER_H_
