/****************************************************************************
 * apps/crypto/openssl_mbedtls_wrapper/include/openssl/err.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 ****************************************************************************/

#ifndef OPENSSL_MBEDTLS_WRAPPER_ERR_H
#define OPENSSL_MBEDTLS_WRAPPER_ERR_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stddef.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ERR_GET_LIB(l) (int)(((l) >> 24L) & 0x0FFL)
#define ERR_GET_REASON(l) (int)((l)&0xFFFL)
#define ERR_LIB_CIPHER 30

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

unsigned long ERR_peek_last_error(void);
void ERR_error_string_n(unsigned long e, char *buf, size_t len);
void ERR_free_strings(void);
char *ERR_error_string(unsigned long e, char *buf);

#ifdef __cplusplus
}
#endif

#endif /* OPENSSL_MBEDTLS_WRAPPER_ERR_H */
