/**
 * \file
 *
 * \brief aes_alt.c to use SAM AES Hw driver
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

#include "mbedtls/aes.h"

#if defined(MBEDTLS_AES_C)
#if defined(MBEDTLS_AES_ALT)

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "aes.h"

/** Priority of the AES interrupt */
#define AES_PRIO           1

/** AES configuration */
struct aes_config g_aes_cfg;
#if PIC32CX
struct aes_ap_config g_aes_ap_cfg;
#endif

/** Output data */
static uint32_t *spul_output_data;

/* State indicate */
volatile bool b_aes_state;

/**
 * \brief The AES interrupt call back function.
 */
static void aes_int_callback(void)
{
	/* Read the output. */
	aes_read_output_data(AES, spul_output_data);
	b_aes_state = true;
}

void mbedtls_aes_init( mbedtls_aes_context *ctx )
{
	//printf("\r\n===mbedtls_aes_init\r\n");
	/* Clear the mbedtls context */
	memset( ctx, 0, sizeof( mbedtls_aes_context ) );

	/* Enable the AES module. */
#if PIC32CX
	aes_get_config_defaults(&g_aes_cfg, &g_aes_ap_cfg);
	aes_init(AES, &g_aes_cfg, &g_aes_ap_cfg);
#else
	aes_get_config_defaults(&g_aes_cfg);
	aes_init(AES, &g_aes_cfg);
#endif
	aes_enable();

	/* Enable AES interrupt. */
	aes_set_callback(AES, AES_INTERRUPT_DATA_READY, aes_int_callback, AES_PRIO);
}

void mbedtls_aes_free( mbedtls_aes_context *ctx )
{
	//printf("\r\n===mbedtls_aes_free\r\n");
	UNUSED(ctx);
	aes_disable();
}

/*
 * AES key schedule (encryption)
 */
int mbedtls_aes_setkey_enc( mbedtls_aes_context *ctx, const unsigned char *key,
                            unsigned int keybits )
{
	//printf("\r\n===mbedtls_aes_setkey_enc\r\n");
		/* Store the key */
		memcpy(ctx->keys, key, keybits / 8);

		/* Set the key size */
		switch(keybits) {
		case 16 * 8:
			ctx->keySize = AES_KEY_SIZE_128;
			break;
		case 24 * 8:
			ctx->keySize = AES_KEY_SIZE_192;
			break;
		case 32 * 8:
			ctx->keySize = AES_KEY_SIZE_256;
			break;
    default :
        return( MBEDTLS_ERR_AES_INVALID_KEY_LENGTH );
		}

    return( 0 );
}

/*
 * AES key schedule (decryption)
 */
int mbedtls_aes_setkey_dec( mbedtls_aes_context *ctx, const unsigned char *key,
                            unsigned int keybits )
{
	//printf("\r\n===mbedtls_aes_setkey_dec\r\n");
	return mbedtls_aes_setkey_enc( ctx, key, keybits );
}

/*
 * AES-ECB block encryption
 */
void mbedtls_aes_encrypt( mbedtls_aes_context *ctx,
                          const unsigned char input[16],
                          unsigned char output[16] )
{
    volatile uint16_t uc_timeout;
	//printf("\r\n===mbedtls_aes_encrypt\r\n");
	b_aes_state = false;

	/* Configure the AES. */
	g_aes_cfg.encrypt_mode = AES_ENCRYPTION;
	/* g_aes_cfg.key_size set when key received */
	g_aes_cfg.start_mode = AES_AUTO_START;
	g_aes_cfg.opmode = AES_ECB_MODE;
	g_aes_cfg.cfb_size = AES_CFB_SIZE_128;
	g_aes_cfg.lod = false;
#if PIC32CX
	aes_set_config(AES, &g_aes_cfg, &g_aes_ap_cfg);
#else
	aes_set_config(AES, &g_aes_cfg);
#endif

	/* Set the cryptographic key. */
	aes_write_key(AES, (uint32_t const *)ctx->keys);

	/* The initialization vector is not used by the ECB cipher mode. */

	/* Set the pointer to the output data */
	spul_output_data = (uint32_t *)output;

	/* Write the data to be ciphered to the input data registers. */
	aes_write_input_data(AES, (uint32_t const *)input);

	/* Wait for the end of the encryption process. */
    uc_timeout = 10000;
	while ((false == b_aes_state) && uc_timeout) {
		uc_timeout--;
	}

}

/*
 * AES-ECB block decryption
 */
void mbedtls_aes_decrypt( mbedtls_aes_context *ctx,
                          const unsigned char input[16],
                          unsigned char output[16] )
{
    volatile uint16_t uc_timeout;
	//printf("\r\n===mbedtls_aes_decrypt\r\n");
	b_aes_state = false;

	/* Configure the AES. */
	g_aes_cfg.encrypt_mode = AES_DECRYPTION;
	/* g_aes_cfg.key_size set when key received */
	g_aes_cfg.start_mode = AES_AUTO_START;
	g_aes_cfg.opmode = AES_ECB_MODE;
	g_aes_cfg.cfb_size = AES_CFB_SIZE_128;
	g_aes_cfg.lod = false;
#if PIC32CX
	aes_set_config(AES, &g_aes_cfg, &g_aes_ap_cfg);
#else
	aes_set_config(AES, &g_aes_cfg);
#endif

	/* Set the cryptographic key. */
	aes_write_key(AES, (uint32_t const *)ctx->keys);

	/* The initialization vector is not used by the ECB cipher mode. */

	/* Set the pointer to the output data */
	spul_output_data = (uint32_t *)output;

	/* Write the data to be ciphered to the input data registers. */
	aes_write_input_data(AES, (uint32_t const *)input);

	/* Wait for the end of the encryption process. */
    uc_timeout = 10000;
	while ((false == b_aes_state) && uc_timeout) {
		uc_timeout--;
	}
}

/*
 * AES-ECB block encryption/decryption
 */
int mbedtls_aes_crypt_ecb( mbedtls_aes_context *ctx,
                           int mode,
                           const unsigned char input[16],
                           unsigned char output[16] )
{
	//printf("\r\n===mbedtls_aes_crypt_ecb\r\n");
    //ctx->opMode = AES_MODE_ECB;
    if( mode == MBEDTLS_AES_ENCRYPT )
        mbedtls_aes_encrypt( ctx, input, output );
    else
        mbedtls_aes_decrypt( ctx, input, output );


    return( 0 );
}

#endif /* MBEDTLS_AES_ALT */


#endif /* MBEDTLS_AES_C */
