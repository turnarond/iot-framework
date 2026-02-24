/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: lwdistcomm.h Main header file for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_H
#define LWDISTCOMM_H

#include "types.h"
#include "address.h"
#include "message.h"
#include "security.h"
#include "client.h"
#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Version information */
#define LWDISTCOMM_VERSION_MAJOR 1
#define LWDISTCOMM_VERSION_MINOR 0
#define LWDISTCOMM_VERSION_PATCH 0

/* Get version string */
const char *lwdistcomm_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_H */
