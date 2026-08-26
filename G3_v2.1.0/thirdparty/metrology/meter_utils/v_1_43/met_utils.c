/* =================================================================== */
#include "asf.h"
#include "met_utils.h"
/* =================================================================== */

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* ================================================ */
/* Description     ::	bcd chang to bin */
/* Function        ::	met_utils_bcd_to_bin */
/* Input           ::	bcddata */
/* Output          ::	bcddata->bin */
/* Call            ::	none */
/* Effect          :: */
/* =============================================== */
uint8_t met_utils_bcd_to_bin( uint8_t bcddata )
{
	return ((bcddata >> 4) * 10 + (bcddata & 0x0f));
}

/* ================================================ */
/* Description     ::	bin change to bcd */
/* Function        ::	met_utils_bin_to_bcd */
/* Input           ::	bindata<0x64 */
/* Output          ::	bindata->bcd */
/* Call            ::	none */
/* Effect          :: */
/* =============================================== */
uint16_t met_utils_bin_to_bcd( uint8_t bindata )
{
	uint16_t i;
	i = (bindata / 100);
	i = i << 4;
	i += (bindata % 100) / 10;
	i = i << 4;
	i += (bindata % 100) % 10;
	return (i);
}

/* ================================================ */
/* Description     ::	two bits bcd data add */
/* Function        ::	met_utils_bcd_to_add */
/* Input           ::	addend,ptr */
/* Output          ::	(addend+*(ptr))->*(ptr) */
/*		   ::	carry */
/* Call            ::	met_utils_bcd_to_bin,met_utils_bin_to_bcd */
/* Effect          :: */
/* =============================================== */
uint8_t met_utils_bcd_to_add( uint8_t addend, uint8_t *point )
{
	uint8_t i;
	uint8_t j;
	uint8_t carry = 0;
	i = met_utils_bcd_to_bin( addend );
	j = met_utils_bcd_to_bin( *point );
	j = i + j;
	if (j >= 100) {
		carry = 1;
		j = j - 100;
	}

	*point = (uint8_t)met_utils_bin_to_bcd( j );
	return (carry);
}

/* ================================================ */
/* Description     ::	eight bits bcd data add */
/* Function        ::	met_utils_bcd_8bits_add */
/* Input           ::	*pointaddend,*pointaugend */
/* Output          ::	result->*pointend,carry */
/* Call            ::	met_utils_bcd_to_bin,met_utils_bin_to_bcd */
/* Effect          :: */
/* =============================================== */
uint8_t met_utils_bcd_8bits_add( uint8_t *pointadd, uint8_t *pointend )
{
	uint8_t carry = 0;
	uint8_t i;
	uint8_t j;
	for (i = 0; i < 4; i++) {
		carry = met_utils_bcd_to_add( *(pointadd + i), (pointend + i));
		if (carry == 1) {
			for (j = i + 1; j < 4; j++) {
				carry = met_utils_bcd_to_add( 1, (pointend + j));
				if (carry == 0) {
					break;
				}
			}
		}
	}
	return (carry);
}

/* =================================================================== */
/* description	        ::	4 bytes hex change to bcd */
/* function		::	met_utils_hex_4byte_to_bcd */
/* input		::	val (hex) */
/* output		::	(bcd) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint64_t met_utils_hex_4byte_to_bcd( uint32_t val )
{
	uint32_t m;
	uint64_t i;
	m = val;
	i = (uint64_t)(m / 1000000000);
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

/* =================================================================== */
/* description	        ::	4 bytes bcd change to hex */
/* function		::	met_utils_bcd_4byte_to_hex */
/* input		::	val (bcd) */
/* output		::	(hex) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t        met_utils_bcd_4byte_to_hex( uint32_t val )
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

uint8_t met_utils_dec_to_char( uint8_t decimal )
{
	uint8_t value = 0;

	switch (decimal) {
	case 0:
		value = 0x30;
		break;

	case 1:
		value = 0x31;
		break;

	case 2:
		value = 0x32;
		break;

	case 3:
		value = 0x33;
		break;

	case 4:
		value = 0x34;
		break;

	case 5:
		value = 0x35;
		break;

	case 6:
		value = 0x36;
		break;

	case 7:
		value = 0x37;
		break;

	case 8:
		value = 0x38;
		break;

	case 9:
		value = 0x39;
		break;

	default:
		break;
	}

	return value;
}

void met_utils_copy_double_to_buffer( uint32_t valueA, uint32_t valueB, uint32_t valueC, uint8_t *buffer, bool flag )
{
	uint8_t buf[34] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
			   ' ', ' '};
	uint8_t i = 0;

	buf[0] = met_utils_dec_to_char((uint8_t)(valueA / 100000));
	valueA = valueA % 100000;
	buf[1] = met_utils_dec_to_char((uint8_t)(valueA / 10000));
	valueA = valueA % 10000;
	buf[2] = met_utils_dec_to_char((uint8_t)(valueA / 1000));
	buf[3] = 0x2e;
	valueA = valueA % 1000;
	buf[4] = met_utils_dec_to_char((uint8_t)(valueA / 100));
	valueA = valueA % 100;
	buf[5] = met_utils_dec_to_char((uint8_t)(valueA / 10));
	valueA = valueA % 10;
	buf[6] = met_utils_dec_to_char((uint8_t)(valueA));
	if (flag == 0) {
		buf[6] = 0x56;
	} else {
		buf[7] = 0x41;
	}

	buf[12] = met_utils_dec_to_char((uint8_t)(valueB / 100000));
	valueB = valueB % 100000;
	buf[13] = met_utils_dec_to_char((uint8_t)(valueB / 10000));
	valueB = valueB % 10000;
	buf[14] = met_utils_dec_to_char((uint8_t)(valueB / 1000));
	buf[15] = 0x2e;
	valueB = valueB % 1000;
	buf[16] = met_utils_dec_to_char((uint8_t)(valueB / 100));
	valueB = valueB % 100;
	buf[17] = met_utils_dec_to_char((uint8_t)(valueB / 10));
	valueB = valueB % 10;
	buf[18] = met_utils_dec_to_char((uint8_t)(valueB));
	if (flag == 0) {
		buf[18] = 0x56;
	} else {
		buf[19] = 0x41;
	}

	buf[24] = met_utils_dec_to_char((uint8_t)(valueC / 100000));
	valueC = valueC % 100000;
	buf[25] = met_utils_dec_to_char((uint8_t)(valueC / 10000));
	valueC = valueC % 10000;
	buf[26] = met_utils_dec_to_char((uint8_t)(valueC / 1000));
	buf[27] = 0x2e;
	valueC = valueC % 1000;
	buf[28] = met_utils_dec_to_char((uint8_t)(valueC / 100));
	valueC = valueC % 100;
	buf[29] = met_utils_dec_to_char((uint8_t)(valueC / 10));
	valueC = valueC % 10;
	buf[30] = met_utils_dec_to_char((uint8_t)(valueC));
	if (flag == 0) {
		buf[30] = 0x56;
	} else {
		buf[31] = 0x41;
	}

	for (i = 0; i < 34; i++) {
		*buffer++ = buf[i];
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
