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

#ifndef _CIPHER_WRAPPER_H
#define _CIPHER_WRAPPER_H

#if defined(__cplusplus)
extern "C"
{
#endif

#define RETURN_GOOD 0

/* 
 * API functions to map on source file to Crypto library used
 * Provided implementation uses mbedTLS
 * User is free to build their own wrapper to any other library
 */

/***********************************************************************************************************************
 * Cipher setup funtion. Called before any other *_cmac_* function for context initialization.
 * This function prepares a cipher context for use with the other cipher primitives.
 * This wrapper function implicitly uses CIPHER_AES_128_ECB mode, which is the one set for G3 EAP-PSK protocol.
 ***********************************************************************************************************************
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_cipher_setup(void);

/***********************************************************************************************************************
 * This function sets the CMAC key, and prepares to authenticate the input data.
 * Must be called with an initialized cipher context.
 ***********************************************************************************************************************
 * @param key The CMAC key.
 * @param keybits The length of the CMAC key in bits.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_cipher_cmac_starts(const unsigned char *key, size_t keybits);

/***********************************************************************************************************************
 * This function feeds an input buffer into an ongoing CMAC computation.
 ***********************************************************************************************************************
 * @param input The buffer holding the input data.
 * @param ilen The length of the input data.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_cipher_cmac_update(const unsigned char *input, size_t ilen);

/***********************************************************************************************************************
 * This function finishes the CMAC operation, and writes the result to the output buffer.
 ***********************************************************************************************************************
 * @param output The output buffer for the CMAC checksum result.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_cipher_cmac_finish(unsigned char *output);

/***********************************************************************************************************************
 * This function prepares the authentication of another message with the same key as the previous CMAC operation.
 ***********************************************************************************************************************
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_cipher_cmac_reset(void);

/***********************************************************************************************************************
 * This function frees and clears the cipher-specific context
 ***********************************************************************************************************************
 * @param m_u8Status The status of the request.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
void cipher_wrapper_cipher_free(void);

/***********************************************************************************************************************
 * This function initializes the specified CCM context, to make references valid, and prepare the context
 * for _ccm_setkey() or _ccm_free().
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void cipher_wrapper_ccm_init(void);

/***********************************************************************************************************************
 * This function initializes the CCM context sets the encryption key.
 ***********************************************************************************************************************
 * @param key The encryption key.
 * @param key_len The key size in bytes.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_ccm_setkey(const unsigned char *key, unsigned int key_len);

/***********************************************************************************************************************
 * This function performs a CCM authenticated decryption of a buffer.
 ***********************************************************************************************************************
 * @param length The length of the input data in Bytes.
 * @param iv The initialization vector (nonce).
 * @param iv_len The length of the nonce in Bytes.
 * @param add The additional data field.
 * @param add_len The length of additional data in Bytes.
 * @param input The buffer holding the input data.
 * @param output The buffer holding the output data.
 * @param tag The buffer holding the authentication field.
 * @param tag_len The length of the authentication field in Bytes
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_ccm_auth_decrypt(size_t length, const unsigned char *iv,
		size_t iv_len, const unsigned char *add, size_t add_len, const unsigned char *input,
		unsigned char *output, const unsigned char *tag, size_t tag_len);

/***********************************************************************************************************************
 * This function encrypts a buffer using CCM.
 ***********************************************************************************************************************
 * @param length The length of the input data in Bytes.
 * @param iv The initialization vector (nonce).
 * @param iv_len The length of the nonce in Bytes.
 * @param add The additional data field.
 * @param add_len The length of additional data in Bytes.
 * @param input The buffer holding the input data.
 * @param output The buffer holding the output data.
 * @param tag The buffer holding the authentication field.
 * @param tag_len The length of the authentication field to generate in Bytes
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_ccm_encrypt_and_tag(size_t length, const unsigned char *iv,
		size_t iv_len, const unsigned char *add, size_t add_len, const unsigned char *input,
		unsigned char *output, unsigned char *tag, size_t tag_len);

/***********************************************************************************************************************
 * This function releases and clears the specified CCM context and underlying cipher sub-context.
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void cipher_wrapper_ccm_free(void);


/***********************************************************************************************************************
 * This function initialises mode and sets key and prepares EAX context for other _eax_* functions.
 ***********************************************************************************************************************
 * @param key The key value.
 * @param key_len The key size in bytes.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_eax_init_and_key(const unsigned char *key, size_t key_len);

/***********************************************************************************************************************
 * This function encrypts a message using EAX context created upon cipher_wrapper_eax_init_and_key() function.
 ***********************************************************************************************************************
 * @param iv The initialization vector (nonce).
 * @param iv_len The length of the nonce in Bytes.
 * @param hdr The header buffer.
 * @param hdr_len The length of header buffer in Bytes.
 * @param msg The message buffer.
 * @param msg_len The length of message buffer in Bytes.
 * @param tag The buffer for the tag.
 * @param tag_len The length of buffer for the tag in Bytes
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_eax_encrypt_message(const unsigned char *iv, size_t iv_len,
		const unsigned char *hdr, size_t hdr_len, unsigned char *msg, size_t msg_len,
		unsigned char *tag, size_t tag_len);

/***********************************************************************************************************************
 * This function decrypts a message using EAX context created upon cipher_wrapper_eax_init_and_key() function.
 ***********************************************************************************************************************
 * @param iv The initialization vector (nonce).
 * @param iv_len The length of the nonce in Bytes.
 * @param hdr The header buffer.
 * @param hdr_len The length of header buffer in Bytes.
 * @param msg The message buffer.
 * @param msg_len The length of message buffer in Bytes.
 * @param tag The buffer for the tag.
 * @param tag_len The length of buffer for the tag in Bytes
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_eax_decrypt_message(const unsigned char *iv, size_t iv_len,
		const unsigned char *hdr, size_t hdr_len, unsigned char *msg, size_t msg_len,
		const unsigned char *tag, size_t tag_len);

/***********************************************************************************************************************
 * This function cleans up EAX context and ends EAX operation.
 ***********************************************************************************************************************
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int cipher_wrapper_eax_end(void);

#if defined(__cplusplus)
}
#endif

#endif
