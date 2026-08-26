/**
 * \file
 *
 * \brief aes_wrapper : Wrapper layer between G3 stack and AES module
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "aes_wrapper.h"
#include "mbedtls/aes.h"
#include "mbedtls/memory_buffer_alloc.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/** Key in byte format */
static uint8_t spuc_key[256];
/** Key size in bits */
static uint16_t sus_key_size;

/** Context used in aes_* functions in this wrapper */
mbedtls_aes_context aes_ctx;
/** Context used in aes_wrapper_* functions in this wrapper */
mbedtls_aes_context aes_wrapper_ctx;

/** buffer to allocate size on crypto_init function */
unsigned char mbedtls_buf[512];

void crypto_init(void)
{
	mbedtls_memory_buffer_alloc_init(mbedtls_buf, sizeof(mbedtls_buf));
}

int aes_encrypt(const unsigned char *in, unsigned char *out)
{
	/* Initialize the AES */
	mbedtls_aes_init(&aes_ctx);

	/* Trigger the AES */
	mbedtls_aes_setkey_enc(&aes_ctx, spuc_key, sus_key_size);
	mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, in, out);

	/* Free the AES */
	mbedtls_aes_free(&aes_ctx);

	return 0;
}


int aes_decrypt(const unsigned char *in, unsigned char *out)
{
	/* Initialize the AES */
	mbedtls_aes_init(&aes_ctx);

	/* Trigger the AES */
	mbedtls_aes_setkey_dec(&aes_ctx, spuc_key, sus_key_size);
	mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, in, out);

	/* Free the AES */
	mbedtls_aes_free(&aes_ctx);

	return 0;
}

int aes_key(const unsigned char *key, unsigned int key_len)
{
	/* Store the key */
	memcpy(spuc_key, key, key_len);

	/* Store the key size */
	sus_key_size = key_len * 8;

	return 0;
}

void aes_wrapper_aes_init(void)
{
	mbedtls_aes_init(&aes_wrapper_ctx);
}

int aes_wrapper_aes_setkey_enc(const unsigned char *key, unsigned int keybits)
{
	return mbedtls_aes_setkey_enc(&aes_wrapper_ctx, key, keybits);
}

int aes_wrapper_aes_encrypt(const unsigned char *in, unsigned char *out)
{
	return mbedtls_aes_crypt_ecb(&aes_wrapper_ctx, MBEDTLS_AES_ENCRYPT, in, out);
}

void aes_wrapper_aes_free(void)
{
	mbedtls_aes_free(&aes_wrapper_ctx);
}

#if defined(__cplusplus)
}
#endif
