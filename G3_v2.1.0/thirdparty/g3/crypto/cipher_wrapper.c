/**
 * \file
 *
 * \brief aes_wrapper : Wrapper layer between G3 stack and Cipher module
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
#include "cipher_wrapper.h"
#include "aes_wrapper.h"
#include "eax.h"
#include "mbedtls/cipher.h"
#include "mbedtls/cmac.h"
#include "mbedtls/ccm.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/*
 * Cipher context initialized upon cipher_wrapper_cipher_setup call
 * and used on subsequent wrapper function calls such as:
 * - cipher_wrapper_cipher_cmac_starts
 * - cipher_wrapper_cipher_cmac_update
 * - cipher_wrapper_cipher_cmac_finish
 * - cipher_wrapper_cipher_cmac_reset
 * and released upon cipher_wrapper_cipher_free call
 */
mbedtls_cipher_context_t mbed_cipher_ctx;

/*
 * CCM context initialized upon cipher_wrapper_ccm_init call
 * and used on subsequent wrapper function calls such as:
 * - cipher_wrapper_ccm_setkey
 * - cipher_wrapper_ccm_encrypt_and_tag
 * - cipher_wrapper_ccm_auth_decrypt
 * and released upon cipher_wrapper_ccm_free call
 */
mbedtls_ccm_context mbed_ccm_ctx;

/*
 * EAX context initialized upon cipher_wrapper_eax_init_and_key call
 * and used on subsequent wrapper function calls such as:
 * - cipher_wrapper_eax_encrypt_message
 * - cipher_wrapper_eax_decrypt_message
 * and released upon cipher_wrapper_eax_end call
 */
eax_ctx brg_eax_ctx;


 /***********************************************************************************************************************
 * Cipher functions wrapper
 **********************************************************************************************************************/

int cipher_wrapper_cipher_setup(void)
{
	const mbedtls_cipher_info_t *cipher_info;
	/* Get cipher info for type MBEDTLS_CIPHER_AES_128_ECB, the one used in G3 EAP-PSK protocol */
	cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
	/* Setup cipher using info */
	return mbedtls_cipher_setup(&mbed_cipher_ctx, cipher_info);
}

int cipher_wrapper_cipher_cmac_starts(const unsigned char *key, size_t keybits)
{
	return mbedtls_cipher_cmac_starts(&mbed_cipher_ctx, key, keybits);
}

int cipher_wrapper_cipher_cmac_update(const unsigned char *input, size_t ilen)
{
	return  mbedtls_cipher_cmac_update(&mbed_cipher_ctx, input, ilen);
}

int cipher_wrapper_cipher_cmac_finish(unsigned char *output)
{
	*output = 0; // To avoid Static Analysis warning
	return mbedtls_cipher_cmac_finish(&mbed_cipher_ctx, output);
}

int cipher_wrapper_cipher_cmac_reset(void)
{
	return mbedtls_cipher_cmac_reset(&mbed_cipher_ctx);
}

void cipher_wrapper_cipher_free(void)
{
	mbedtls_cipher_free(&mbed_cipher_ctx);
}


 /***********************************************************************************************************************
 * CCM functions wrapper
 **********************************************************************************************************************/

void cipher_wrapper_ccm_init(void)
{
	mbedtls_ccm_init(&mbed_ccm_ctx);
}

int cipher_wrapper_ccm_setkey(const unsigned char *key, unsigned int key_len)
{
	return mbedtls_ccm_setkey(&mbed_ccm_ctx, MBEDTLS_CIPHER_ID_AES, key, (key_len << 3)); // mbedtls expects key len in bits
}

int cipher_wrapper_ccm_auth_decrypt(size_t length, const unsigned char *iv,
		size_t iv_len, const unsigned char *add, size_t add_len, const unsigned char *input,
		unsigned char *output, const unsigned char *tag, size_t tag_len)
{
	return mbedtls_ccm_auth_decrypt(&mbed_ccm_ctx, length, iv, iv_len, add, add_len, input, output, tag, tag_len);
}

int cipher_wrapper_ccm_encrypt_and_tag(size_t length, const unsigned char *iv,
		size_t iv_len, const unsigned char *add, size_t add_len, const unsigned char *input,
		unsigned char *output, unsigned char *tag, size_t tag_len)
{
	return mbedtls_ccm_encrypt_and_tag(&mbed_ccm_ctx, length, iv, iv_len, add, add_len, input, output, tag, tag_len);
}

void cipher_wrapper_ccm_free(void)
{
	mbedtls_ccm_free(&mbed_ccm_ctx);
}


 /***********************************************************************************************************************
 * EAX functions wrapper
 **********************************************************************************************************************/

int cipher_wrapper_eax_init_and_key(const unsigned char *key, size_t key_len)
{
	return eax_init_and_key(key, key_len, &brg_eax_ctx);
}

int cipher_wrapper_eax_encrypt_message(const unsigned char *iv, size_t iv_len,
		const unsigned char *hdr, size_t hdr_len, unsigned char *msg, size_t msg_len,
		unsigned char *tag, size_t tag_len) {
	return eax_encrypt_message(iv, iv_len, hdr, hdr_len, msg, msg_len, tag, tag_len, &brg_eax_ctx);
}

int cipher_wrapper_eax_decrypt_message(const unsigned char *iv, size_t iv_len,
		const unsigned char *hdr, size_t hdr_len, unsigned char *msg, size_t msg_len,
		const unsigned char *tag, size_t tag_len) {
	return eax_decrypt_message(iv, iv_len, hdr, hdr_len, msg, msg_len, tag, tag_len, &brg_eax_ctx);
}

int cipher_wrapper_eax_end(void) {
	return eax_end(&brg_eax_ctx);
}

#if defined(__cplusplus)
}
#endif
