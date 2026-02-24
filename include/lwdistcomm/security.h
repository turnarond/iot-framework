/*
 * Copyright (c) 2026 ACOAUTO Team.
 * All rights reserved.
 *
 * Detailed license information can be found in the LICENSE file.
 *
 * File: security.h Security functions for LwDistComm.
 *
 */

#ifndef LWDISTCOMM_SECURITY_H
#define LWDISTCOMM_SECURITY_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create security context */
lwdistcomm_security_t *lwdistcomm_security_create(const lwdistcomm_security_options_t *options);

/* Set authentication information */
bool lwdistcomm_security_set_auth(lwdistcomm_security_t *security, const lwdistcomm_auth_info_t *auth_info);

/* Enable TLS */
bool lwdistcomm_security_enable_tls(lwdistcomm_security_t *security, const char *cert_file, const char *key_file);

/* Verify peer certificate */
bool lwdistcomm_security_verify_cert(lwdistcomm_security_t *security, const char *ca_file);

/* Encrypt data */
bool lwdistcomm_security_encrypt(lwdistcomm_security_t *security, const void *input, size_t input_len, void *output, size_t *output_len);

/* Decrypt data */
bool lwdistcomm_security_decrypt(lwdistcomm_security_t *security, const void *input, size_t input_len, void *output, size_t *output_len);

/* Check if TLS is enabled */
bool lwdistcomm_security_is_tls_enabled(const lwdistcomm_security_t *security);

/* Destroy security context */
void lwdistcomm_security_destroy(lwdistcomm_security_t *security);

#ifdef __cplusplus
}
#endif

#endif /* LWDISTCOMM_SECURITY_H */
