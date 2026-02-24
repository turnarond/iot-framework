/*
* Copyright (c) 2024 ACOINFO CloudNative Team.
* All rights reserved.
*
* Detailed license information can be found in the LICENSE file.
*
* File: version.h .
*
* Date: 2024-05-08
*
* Author: Yan.chaodong <yanchaodong@acoinfo.com>
*
* 2024.05.20  Update version to 1.1.0, Add gateway function.
* 2024.07.15  Update version to 1.1.1. Add vsoa_log, authority; 
                                       Add rpc multi listeners.
                                       Add rpc set/get function. 
                                       Fix some known bugs.
* 2024.07.15  Update version to 1.2.0. Add datagram function; 
                                       Add restful API;
                                       Add ecs_guard.
                                       Fix some known bugs.
* 2024.07.15  Update version to 1.2.1. Update some callback functions of client with std::function.
* 2025.05.25  Update version to 1.3.1. Update vsoa_logger with sqlite3.
* 2025.07.29  Update version to 1.3.2. Add vsoa_master help url /help.
* 2025.09.22  Update version to 1.3.3. Add some unit test. Fix some bugs.
* 2025.11.03  Update version to 1.3.4. Fix some knowns bugs.
* 2025.11.13  Update version to 1.3.5. Add Resubscribe of urls after client reconnect to server.
*/

#ifndef VSOAPLATFORM_SDK_VERSION_H
#define VSOAPLATFORM_SDK_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define VSOA_PLATFORM_MAJOR_VER 1
#define VSOA_PLATFORM_MINOR_VER 3
#define VSOA_PLATFORM_PATCH_VER 5

#define VSOA_PLATFORM_STRXCHG(x)        #x
#define VSOA_PLATFORM_MAKEVER(a, b, c)  (((a) << 16) + ((b) << 8) + (c))

#define VSOA_PLATFORM_MAKEVERSTR(a, b, c)    \
        VSOA_PLATFORM_STRXCHG(a) "." VSOA_PLATFORM_STRXCHG(b) "." VSOA_PLATFORM_STRXCHG(c)

#define VSOA_PLATFORM_VERSION   VSOA_PLATFORM_MAKEVER(VSOA_PLATFORM_MAJOR_VER, \
                                                      VSOA_PLATFORM_MINOR_VER, \
                                                      VSOA_PLATFORM_PATCH_VER)

#define VSOA_PLATFORM_VERSTR    VSOA_PLATFORM_MAKEVERSTR(VSOA_PLATFORM_MAJOR_VER, \
                                                         VSOA_PLATFORM_MINOR_VER, \
                                                         VSOA_PLATFORM_PATCH_VER)

void DumpVersionInfo(void);

#ifdef __cplusplus
}
#endif
#endif // VSOAPLATFORM_SDK_VERSION_H