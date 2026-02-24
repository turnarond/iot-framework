/***************************************************************************
 *
 * Project VSOA PLATFORM SDK
 *
 *
 * Copyright  SylixOS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef vsoa_concurrency_SpinLock_hpp
#define vsoa_concurrency_SpinLock_hpp

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS
#include <atomic>

namespace vsoa { namespace concurrency {

/**
 * SpinLock implementation based on atomic.
 * Meets the `Lockable` requirements.
 */
class SpinLock {
protected:
  std::atomic<bool> m_atom;
public:

  /**
   * Constructor.
   */
  SpinLock();

  /**
   * Lock spin-lock
   */
  void lock();

  /**
   * Unlock spin-lock
   */
  void unlock();

  /**
   * Try to lock.
   * @return - `true` if the lock was acquired, `false` otherwise.
   */
  bool try_lock();

};
  
}}

#endif /* vsoa_concurrency_SpinLock_hpp */
