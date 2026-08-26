/* =================================================================== */
#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "compiler.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */
uint8_t met_utils_bcd_to_bin( uint8_t bcddata );
uint16_t met_utils_bin_to_bcd( uint8_t bindata );
uint8_t met_utils_bcd_to_add( uint8_t addend, uint8_t *point );
uint8_t met_utils_bcd_8bits_add( uint8_t *pointadd, uint8_t *pointend );
uint64_t met_utils_hex_4byte_to_bcd( uint32_t val );
uint32_t met_utils_bcd_4byte_to_hex( uint32_t val );
uint8_t met_utils_dec_to_char( uint8_t decimal );
void met_utils_copy_double_to_buffer( uint32_t valueA, uint32_t valueB, uint32_t valueC, uint8_t *buffer, bool flag );

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* UTILS_H_INCLUDED */
