/**
 * \file
 *
 * \brief Meter Demo : Utils module
 *
 * Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "string.h"
#include "sysclk.h"
#include "utils.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/**
 * \brief 4 bytes hex change to BCD.
 *
 * \param val Hex value
 *
 * \return BCD value
 */
uint32_t Utils4HexToBCD(uint32_t val)
{
	uint32_t m;
	uint32_t i;

	m = val;
	i = (m / 1000000000);
	i = i << 4;
	i |= ((m % 1000000000) / 100000000);
	i = i << 4;
	i |= ((m % 100000000) / 10000000);
	i = i << 4;
	i |= ((m % 10000000) / 1000000);
	i = i << 4;
	i |= ((m % 1000000) / 100000);
	i = i << 4;
	i |= ((m % 100000) / 10000);
	i = i << 4;
	i |= ((m % 10000) / 1000);
	i = i << 4;
	i |= ((m % 1000) / 100);
	i = i << 4;
	i |= ((m % 100) / 10);
	i = i << 4;
	i |= ((m % 10) / 1);

	return (i);
}

/**
 * \brief 4 bytes bcd change to hex.
 *
 * \param val BCD value
 *
 * \return HEX value
 */
uint32_t Utils4BCDToHex(uint32_t val)
{
	uint32_t i, m;

	i = 0;
	m = val;
	i = (m >> 28) * 10;
	i = (i + ((m >> 24) & 0x0F)) * 10;
	i = (i + ((m >> 20) & 0x0F)) * 10;
	i = (i + ((m >> 16) & 0x0F)) * 10;
	i = (i + ((m >> 12) & 0x0F)) * 10;
	i = (i + ((m >> 8) & 0x0F)) * 10;
	i = (i + ((m >> 4) & 0x0F)) * 10;
	i = i + (m & 0x0F);

	return (i);
}

/**
 * \brief BCD change to BIN.
 *
 * \param bcddata BCD value
 *
 * \return BIN value
 */
uint8_t UtilsBCDToBIN(uint8_t bcddata)
{
	return ((bcddata >> 4) * 10 + (bcddata & 0x0f));
}

/**
 * \brief BIN change to BCD.
 *
 * \param bcddata BIN value
 *
 * \return BCD value
 */
uint16_t UtilsBINToBCD(uint8_t bindata)
{
	uint16_t i;

	i = (bindata / 100);
	i = i << 4;
	i += (bindata % 100) / 10;
	i = i << 4;
	i += (bindata % 100) % 10;

	return (i);
}

/**
 * \brief Two bits BCD data add.
 *
 * \param addend BCD value
 * \param point Pointer to the second BCD value to add, and then
 *	it is used to store the result
 *
 * \return Carry
 */
uint8_t UtilsBCD2Add(uint8_t addend, uint8_t *point)
{
	uint8_t i;
	uint8_t j;
	uint8_t carry = 0;

	i = UtilsBCDToBIN(addend);
	j = UtilsBCDToBIN(*point);
	j = i + j;

	if (j >= 100) {
		carry = 1;
		j = j - 100;
	}

	*point = (uint8_t)UtilsBINToBCD(j);

	return (carry);
}

/**
 * \brief Eight bits BCD data add.
 *
 * \param pointadd Pointer to the first BCD value
 * \param pointend Pointer to the second BCD value to add, and then
 *	it is used to store the result
 *
 * \return Carry
 */
uint8_t UtilsBCD8Add(uint8_t *pointadd, uint8_t *pointend)
{
	uint8_t carry = 0;
	uint8_t i;
	uint8_t j;

	for (i = 0; i < 4; i++) {
		carry = UtilsBCD2Add(*(pointadd + i), (pointend + i));
		if (carry == 1) {
			for (j = i + 1; j < 4; j++) {
				carry = UtilsBCD2Add(1, (pointend + j));
				if (carry == 0) {
					break;
				}
			}
		}
	}

	return (carry);
}

/**
 * \brief Write string to memory buffer.
 *
 * \param sptr Pointer to the string
 * \param mptr Pointer to the buffer to write the string
 *
 * \return Length of the string.
 */
uint32_t UtilsWriteStringToBuffer(const uint8_t *sptr, uint8_t *mptr)
{
	uint32_t i, j;

	j = 0;
	for (i = 0; i < 36; i++) {
		if ((*(sptr + i)) == NULL) {
			break;
		}

		*(mptr + i) = *(sptr + i);
		j++;
	}

	return (j);
}

/**
 * \brief Write Values in 32bit to memory buffer.
 *
 * \param valueA First value to write in buffer
 * \param valueB Second value to write in buffer
 * \param valueC Third value to write in buffer
 * \param buffer Pointer to the buffer to write the string
 * \param flag   Flag to ??
 *
 * \return Length of the string.
 */
void UtilsWriteDoubleToBuffer(uint32_t valueA, uint32_t valueB, uint32_t valueC,
				 uint8_t *buffer, bool flag)
{
	uint8_t buf[34] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' '};
	uint8_t i = 0;

	buf[0] = DECTOCHAR((uint8_t)(valueA / 100000));
	valueA = valueA % 100000;
	buf[1] = DECTOCHAR((uint8_t)(valueA / 10000));
	valueA = valueA % 10000;
	buf[2] = DECTOCHAR((uint8_t)(valueA / 1000));
	buf[3] = 0x2e;
	valueA = valueA % 1000;
	buf[4] = DECTOCHAR((uint8_t)(valueA / 100));
	valueA = valueA % 100;
	buf[5] = DECTOCHAR((uint8_t)(valueA / 10));
	valueA = valueA % 10;
	buf[6] = DECTOCHAR((uint8_t)(valueA));
	if (flag == 0) {
		buf[6] = 0x56;
	} else {
		buf[7] = 0x41;
	}

	buf[12] = DECTOCHAR((uint8_t)(valueB / 100000));
	valueB = valueB % 100000;
	buf[13] = DECTOCHAR((uint8_t)(valueB / 10000));
	valueB = valueB % 10000;
	buf[14] = DECTOCHAR((uint8_t)(valueB / 1000));
	buf[15] = 0x2e;
	valueB = valueB % 1000;
	buf[16] = DECTOCHAR((uint8_t)(valueB / 100));
	valueB = valueB % 100;
	buf[17] = DECTOCHAR((uint8_t)(valueB / 10));
	valueB = valueB % 10;
	buf[18] = DECTOCHAR((uint8_t)(valueB));
	if (flag == 0) {
		buf[18] = 0x56;
	} else {
		buf[19] = 0x41;
	}

	buf[24] = DECTOCHAR((uint8_t)(valueC / 100000));
	valueC = valueC % 100000;
	buf[25] = DECTOCHAR((uint8_t)(valueC / 10000));
	valueC = valueC % 10000;
	buf[26] = DECTOCHAR((uint8_t)(valueC / 1000));
	buf[27] = 0x2e;
	valueC = valueC % 1000;
	buf[28] = DECTOCHAR((uint8_t)(valueC / 100));
	valueC = valueC % 100;
	buf[29] = DECTOCHAR((uint8_t)(valueC / 10));
	valueC = valueC % 10;
	buf[30] = DECTOCHAR((uint8_t)(valueC));
	if (flag == 0) {
		buf[30] = 0x56;
	} else {
		buf[31] = 0x41;
	}

	for (i = 0; i < 34; i++) {
		*buffer++ = buf[i];
	}
}

/**
 * \brief Change Hex To Char
 *
 * \param val Hex value
 * \param ptr Destination pointer
 *
 * \retval len<=16
 *
 */
uint32_t UtilsHexToChar(uint64_t val, uint8_t *ptr)
{
	uint8_t n;
	uint32_t i, j, len;
	uint64_t m, k;
	m = val;
	len = 0;
	k = 0xF000000000000000;
	j = 60;
	len = 0;
	for (i = 0; i < 16; i++) {
		n = (uint8_t)((m & k) >> j);
		if ((n != 0) || (len != 0) || (i == 15)) {
			if (n < 10) {
				*(ptr + len) = (n + '0');
			}

			if (n > 9) {
				*(ptr + len) = (n + '7');
			}

			len++;
		}

		k = k >> 4;
		j = j - 4;
	}
	return (len);
}

/**
 * \brief give up continuous char data high bit zero
 *
 * \param sptr Pointer to source
 * \param charnum
 *
 * \retval  len active char num >=1
 *
 */
uint32_t UtilsGiveUpNCharHighZero(uint8_t *sptr, uint8_t charnum)
{
	uint8_t i;
	uint32_t len = 0;
	for (i = 0; i < charnum; i++) {
		if (ISHEX( *(sptr + i))) {
			if (*(sptr + i) != '0' || (*(sptr + i) == '0' && (len != 0 || len == (charnum - 1)))) {
				*(sptr + len) = *(sptr + i);
				len++;
			}
		}
	}
	return (len);
}

/**
 * \brief ASCII combine byte data
 *
 * \param sptr Pointer to source
 * \param mptr Pointer to destination
 * \param len length
 *
 * \retval  len byte num
 *
 */
uint32_t UtilsASCIICombineByteData(uint8_t *sptr, uint8_t *mptr, uint8_t len)
{
	uint8_t i;
	for (i = 0; i < (len >> 1); i++) {
		*(mptr + i) = ((*(sptr + (i << 1) + 1)) << 4) + *(sptr + (i << 1));
	}
	if ((len & 0x01) == 0x01) {
		*(mptr + (len >> 1)) = *(sptr + len - 1);
	}

	return ((len >> 1) + (len & 0x01));
}

/**
 * \brief copy n byte data in inverted order
 *
 * \param sptr Pointer to source
 * \param mptr Pointer to destination
 * \param len length
 *
 * \retval  len byte num
 *
 */
uint32_t UtilsCopyByteDataAndInvertOrder(uint8_t *sptr, uint8_t *mptr, uint8_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++) {
		*(mptr + len - 1 - i) = *(sptr + i);
	}
	return (i);
}

/**
 * \brief copy byte data from sptr to mptr
 *
 * \param sptr Source pointer
 * \param mptr Destination pointer
 * \param len Length
 */
void UtilsCopyByteData(uint8_t *sptr, uint8_t *mptr, uint8_t len)
{
    uint8_t	i;
    for ( i = 0; i < len; i++ )
    {
        *( mptr + i ) = *( sptr + i );
    }
}

/**
 * \brief change N byte hex data to char data
 *
 * \param sptr Pointer to source (hex)
 * \param mptr Pointer to destination (char)
 * \param len length
 *
 * \retval  len byte num
 *
 */
uint32_t UtilsNByteHexToChar(uint8_t *sptr, uint8_t *mptr, uint8_t len)
{
	uint8_t i, n;
	for (i = 0; i < len; i++) {
		n = *(sptr + i);
		if ((n & 0x0F) < 10) {
			*(mptr + (i << 1)) = (n & 0x0F) + '0';
		} else {
			*(mptr + (i << 1)) = (n & 0x0F) + '7';
		}

		if ((n >> 4) < 10) {
			*(mptr + (i << 1) + 1) = (n >> 4) + '0';
		} else {
			*(mptr + (i << 1) + 1) = (n >> 4) + '7';
		}
	}
	return ((uint32_t)i);
}

void UtilsMemCpy(void *pv_dst, void *pv_src, uint16_t us_size)
{
	uint16_t xtf_size;

	/* Set Critical region */
	__disable_irq();

	/* Enable PMC clock */
	sysclk_enable_peripheral_clock(ID_MEM2MEM0);

	/* Configure Transfer Size */
	if (!(us_size & 0x03)) {
		MEM2MEM0->MEM2MEM_MR = MEM2MEM_MR_TSIZE_T_32BIT;
		xtf_size = us_size >> 2;
	} else if (!(us_size & 0x01)) {
		MEM2MEM0->MEM2MEM_MR = MEM2MEM_MR_TSIZE_T_16BIT;
		xtf_size = us_size >> 1;
	} else {
		MEM2MEM0->MEM2MEM_MR = MEM2MEM_MR_TSIZE_T_8BIT;
		xtf_size = us_size;
	}

	/* Prepare MEM2MEM transfer */
	MEM2MEM0->MEM2MEM_TPR = (uint32_t)pv_src;
	MEM2MEM0->MEM2MEM_TCR = xtf_size;
	MEM2MEM0->MEM2MEM_RPR = (uint32_t)pv_dst;
	MEM2MEM0->MEM2MEM_RCR = xtf_size;


	/* Start PDC transfer */
	MEM2MEM0->MEM2MEM_PTCR = (MEM2MEM_PTCR_RXTEN | MEM2MEM_PTCR_TXTEN);
	/* Wait until transfer done */
	while (0 == (MEM2MEM0->MEM2MEM_ISR & MEM2MEM_ISR_RXEND)) {
	}
	/* Stop PDC transfer */
	MEM2MEM0->MEM2MEM_PTCR = (MEM2MEM_PTCR_RXTDIS | MEM2MEM_PTCR_TXTDIS);

	/* Disable PMC clock */
	sysclk_disable_peripheral_clock(ID_MEM2MEM0);

	/* Clear Critical region */
	__enable_irq();
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
