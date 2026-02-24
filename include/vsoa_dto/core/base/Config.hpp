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

/**
 * This is list of default configuration params and values which you can
 * configure in the build-time
 */

#ifndef vsoa_base_Config_hpp
#define vsoa_base_Config_hpp

/**
 * If NOT DISABLED, counting of all object of class vsoa::base::Countable is enabled
 * for debug purposes and detection of memory leaks.
 * Disable object counting for Release builds using '-D VSOA_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance
 */
//#define VSOA_DISABLE_ENV_OBJECT_COUNTERS

/**
 * Define this to disable memory-pool allocations.
 * This will make vsoa::base::memory::MemoryPool, method obtain and free call new and delete directly
 */
//#define VSOA_DISABLE_POOL_ALLOCATIONS

/**
 * Predefined value for function vsoa::concurrency::Thread::getHardwareConcurrency();
 */
//#define VSOA_THREAD_HARDWARE_CONCURRENCY 4

/**
 * Number of shards of ThreadDistributedMemoryPool (Default pool for many vsoa objects)
 * Higher number reduces threads racing for resources on each shard.
 */
#ifndef VSOA_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT
  #define VSOA_THREAD_DISTRIBUTED_MEM_POOL_SHARDS_COUNT 10
#endif

//#define VSOA_COMPAT_BUILD_NO_THREAD_LOCAL 1

#ifndef VSOA_FLOAT_STRING_FORMAT
  #define VSOA_FLOAT_STRING_FORMAT "%.16g"
#endif

/**
 * DISABLE logs priority V
 */
//#define VSOA_DISABLE_LOGV

/**
 * DISABLE logs priority D
 */
//#define VSOA_DISABLE_LOGD

/**
 * DISABLE logs priority I
 */
//#define VSOA_DISABLE_LOGI

/**
 * DISABLE logs priority W
 */
//#define VSOA_DISABLE_LOGW

/**
 * DISABLE logs priority E
 */
//#define VSOA_DISABLE_LOGE


#endif /* vsoa_base_Config_hpp */
