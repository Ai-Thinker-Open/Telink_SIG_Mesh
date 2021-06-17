/**
 * \file mbedtls_sha256.h
 *
 * \brief SHA-224 and SHA-256 cryptographic hash function
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
#ifndef MBEDTLS_SHA256_HKDF_H
#define MBEDTLS_SHA256_HKDF_H

unsigned int mbedtls_sha256_hkdf(const unsigned char *key, unsigned int key_len, const unsigned char *salt, unsigned int salt_len,
                        const unsigned char *info, unsigned int info_len, unsigned char *out, unsigned int out_len);

#endif /* mbedtls_sha256.h */
