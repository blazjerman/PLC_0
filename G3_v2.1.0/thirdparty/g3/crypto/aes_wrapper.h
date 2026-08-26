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

#ifndef _AES_WRAPPER_H
#define _AES_WRAPPER_H

#if defined(__cplusplus)
extern "C"
{
#endif

/***********************************************************************************************************************
 * This function must be called before first use if non-static tables are being used.
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void crypto_init(void);

/***********************************************************************************************************************
 * This function preforms AES encryption using context and key previouly initialized on aes_key() function.
 ***********************************************************************************************************************
 * @param in Pointer to input buffer.
 * @param out Pointer to output buffer where result will be written.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int aes_encrypt(const unsigned char *in, unsigned char *out);

/***********************************************************************************************************************
 * This function preforms AES decryption using context and key previouly initialized on aes_key() function.
 ***********************************************************************************************************************
 * @param in Pointer to input buffer.
 * @param out Pointer to output buffer where result will be written.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int aes_decrypt(const unsigned char *in, unsigned char *out);

/***********************************************************************************************************************
 * This initializes context and key for further encryption and decryption
 * using aes_encrypt() and aes_decrypt() functions.
 ***********************************************************************************************************************
 * @param key Pointer to the key value.
 * @param out The key size in bytes.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int aes_key(const unsigned char *key, unsigned int key_len);


/***********************************************************************************************************************
 * This function initializes wrapper context to be used on functions
 *   aes_wrapper_aes_setkey_enc()
 *   aes_wrapper_aes_encrypt()
 * and release on aes_wrapper_aes_free() function
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void aes_wrapper_aes_init(void);

/***********************************************************************************************************************
 * This function sets the key for further encryption.
 ***********************************************************************************************************************
 * @param key Pointer to the key value.
 * @param keybits The key size in bits.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int aes_wrapper_aes_setkey_enc(const unsigned char *key, unsigned int keybits);

/***********************************************************************************************************************
 * This function preforms AES encryption using context and key previouly initialized on
 * aes_wrapper_aes_init() and aes_wrapper_aes_setkey_enc() functions.
 ***********************************************************************************************************************
 * @param in Pointer to input buffer.
 * @param out Pointer to output buffer where result will be written.
 * @return 0 on Success, otherwise Error Code.
 **********************************************************************************************************************/
int aes_wrapper_aes_encrypt(const unsigned char *in, unsigned char *out);

/***********************************************************************************************************************
 * This function releases the wrapper context that was initialized on aes_wrapper_aes_init() function
 ***********************************************************************************************************************
 **********************************************************************************************************************/
void aes_wrapper_aes_free(void);

#if defined(__cplusplus)
}
#endif

#endif
