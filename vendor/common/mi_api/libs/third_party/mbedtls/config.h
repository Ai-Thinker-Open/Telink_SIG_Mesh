/**
 * \brief Configuration options (set of defines)
 *
 *  This set of compile-time options may be used to enable
 *  or disable features selectively, and reduce the global
 *  memory footprint.
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#define MBEDTLS_CIPHER_MODE_CTR 1

#define MSC_CIPHER_CRT_DER      1
#define MSC_CIPHER_PEM          0
#define MSC_CIPHER_SHA          1
#define MSC_CIPHER_HKDF         1
#define MSC_CIPHER_AES          1
#define MSC_CIPHER_ECDH         0
#define MSC_CIPHER_ECDSA        0

#if (MSC_CIPHER_CRT_DER == 1)
#define MBEDTLS_ASN1_PARSE_C
#endif
#if (MSC_CIPHER_PEM == 1)
#define MBEDTLS_BASE64_C
#endif
#if (MSC_CIPHER_SHA) || (MSC_CIPHER_HKDF)
#define MBEDTLS_SHA256_C
#endif
#if (MSC_CIPHER_HKDF)
#define MBEDTLS_MD_C
#endif

#if (MSC_CIPHER_AES)
#define MBEDTLS_CIPHER_C
#define MBEDTLS_AES_C
#define MBEDTLS_CCM_C
#endif

#if (MSC_CIPHER_ECDSA) || (MSC_CIPHER_ECDH)
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_BIGNUM_C
#endif

#if (MSC_CIPHER_ECDH)
#define MBEDTLS_ECDH_C
#endif

#if (MSC_CIPHER_ECDSA)
#ifndef MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#endif
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_ECDSA_C
#endif

#define MBEDTLS_CMAC_C

#if defined(MBEDTLS_ALT_CONFIG_FILE)
#include MBEDTLS_ALT_CONFIG_FILE
#endif

#include "check_config.h"

#endif /* MBEDTLS_CONFIG_H */
