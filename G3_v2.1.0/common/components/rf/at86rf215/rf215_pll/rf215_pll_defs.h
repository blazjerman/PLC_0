/**
 *
 * \file
 *
 * \brief RF215 Frequency Synthesizer (PLL) definitions.
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

#ifndef RF215_PLL_DEFS_H_INCLUDE
#define RF215_PLL_DEFS_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"
#include "rf215_reg.h"
#include "rf215_phy_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RF215 frequency ranges in Hz [Table 6-21] */
/* First range: 389.5MHz - 510MHz (RF09) */
#define PLL_FREQ_MIN_RF09_RNG1_Hz           389500000
#define PLL_FREQ_MAX_RF09_RNG1_Hz           510000000
/* Second range: 779MHz - 1020MHz (RF09) */
#define PLL_FREQ_MIN_RF09_RNG2_Hz           779000000
#define PLL_FREQ_MAX_RF09_RNG2_Hz          1020000000
/* Third range: 2400MHz - 2483.5MHz (RF24) */
#define PLL_FREQ_MIN_RF24_RNG3_Hz          2400000000
#define PLL_FREQ_MAX_RF24_RNG3_Hz          2483500000

/** IEEE-compliant Scheme (CNM.CM=0) */
/* Frequency resolution: 25kHz */
#define PLL_IEEE_FREQ_STEP_Hz                   25000
/* Maximum channel number (9 bits) */
#define PLL_IEEE_CHN_NUM_MAX                      511
/* Maximum channel spacing: 255 (8 bits) * 25kHz = 6.375MHz*/
#define PLL_IEEE_CHN_SPA_MAX_Hz            (PLL_IEEE_FREQ_STEP_Hz * UINT8_MAX)
/* Frequency offset (RF09): 0Hz */
#define PLL_IEEE_FREQ_OFFSET09_Hz                   0
/* Frequency offset (RF24): 1.5GHz */
#define PLL_IEEE_FREQ_OFFSET24_Hz          1500000000

/** Fine Resolution Channel Scheme (CNM.CM!=0) */
/* Frequency offset (RF09, first range, CNM.CM=1): 377MHz */
#define PLL_FINE_FREQ_OFFSET_RF09_RNG1_Hz   370000000
/* Frequency offset (RF09, second range, CNM.CM=2): 754MHz */
#define PLL_FINE_FREQ_OFFSET_RF09_RNG2_Hz   754000000
/* Frequency offset (RF24, third range, CNM.CM=3): 2366MHz */
#define PLL_FINE_FREQ_OFFSET_RF24_RNG3_Hz  2366000000
/* Frequency resolution (RF09, first range, CNM.CM=1): 6.5MHz / 2^16 */
#define PLL_FINE_FREQ_RES_RF09_RNG1_Hz        6500000
/* Frequency resolution (RF09, second range, CNM.CM=2): 13MHz / 2^16 */
#define PLL_FINE_FREQ_RES_RF09_RNG2_Hz       13000000
/* Frequency resolution (RF24, third range, CNM.CM=3): 26MHz / 2^16 */
#define PLL_FINE_FREQ_RES_RF24_RNG3_Hz       26000000

/** Maximum frequency tolerance definitions */
/* Convert PPM (10^-6) to uQ0.45 */
#define PLL_PPM_TO_Q45(x)                  div_round(x << 45, 1000000)

/* Frequency tolerance for FSK PHY: T<=min(50*10^-6, T0*R*h*F0/R0/h0/F)
 * [IEEE 802.15.4 20.6.3] */
#define PLL_DELTA_FSK_TMAX_Q45             PLL_PPM_TO_Q45(50ULL)
#define PLL_DELTA_FSK_T0_RF09_Q45          PLL_PPM_TO_Q45(30ULL)
#define PLL_DELTA_FSK_T0_RF24_Q45          PLL_PPM_TO_Q45(40ULL)
#define PLL_DELTA_FSK_F0_Hz                 915000000
#define PLL_DELTA_FSK_R0_kHz                       50

/* Frequency tolerance for OFDM PHY. T<=20*10^-6 [IEEE 802.15.4 21.5.9] */
#define PLL_DELTA_OFDM_TMAX_Q45            PLL_PPM_TO_Q45(20ULL)

/** Struct definition for frequency range */
typedef struct pll_freq_rng {
	uint32_t ul_freq_min;
	uint32_t ul_freq_max;
} pll_freq_rng_t;

/** RF09 frequency ranges */
#define PLL_FREQ_RF09_RNG1   {.ul_freq_min = PLL_FREQ_MIN_RF09_RNG1_Hz,	\
			      .ul_freq_max = PLL_FREQ_MAX_RF09_RNG1_Hz}
#define PLL_FREQ_RF09_RNG2   {.ul_freq_min = PLL_FREQ_MIN_RF09_RNG2_Hz,	\
			      .ul_freq_max = PLL_FREQ_MAX_RF09_RNG2_Hz}
#define PLL_FREQ_RNG_RF09    {PLL_FREQ_RF09_RNG1, PLL_FREQ_RF09_RNG2}
/** RF24 frequency range */
#define PLL_FREQ_RF24_RNG3   {.ul_freq_min = PLL_FREQ_MIN_RF24_RNG3_Hz,	\
			      .ul_freq_max = PLL_FREQ_MAX_RF24_RNG3_Hz}
#define PLL_FREQ_RNG_RF24    {PLL_FREQ_RF24_RNG3}

/** Struct definition for RF215 PLL constants */
typedef struct pll_const {
	/* Frequency ranges */
	pll_freq_rng_t px_freq_rng[2];
	/* Frequency resolution for Fine Resolution Channel Scheme (CNM.CM!=0) */
	uint32_t pul_fine_freq_res[2];
	/* Frequency offset for Fine Resolution Channel Scheme (CNM.CM != 0) */
	uint32_t pul_fine_freq_offset[2];
	/* Frequency offset for IEEE-compliant Scheme (CNM.CM = 0) */
	uint32_t ul_ieee_freq_offset;
	/* Frequency tolerance T0 for FSK PHY in uQ0.45 */
	uint32_t ul_fsk_tol_t0;
	/* Fine channel mode for each frequency range (CNM.CM)  */
	uint8_t puc_fine_chn_mode[2];
	/* Number of frequency ranges (2 in RF09, 1 in RF24) */
	uint8_t uc_num_freq_rng;
} pll_const_t;

/** RF09 frequency resolution for fine mode */
#define PLL_FREQ_RES_RF09    {PLL_FINE_FREQ_RES_RF09_RNG1_Hz, \
			      PLL_FINE_FREQ_RES_RF09_RNG2_Hz}
/** RF24 frequency resolution for fine mode */
#define PLL_FREQ_RES_RF24    {PLL_FINE_FREQ_RES_RF24_RNG3_Hz}

/** RF09 frequency offset for fine mode */
#define PLL_FREQ_OFFSET_RF09 {PLL_FINE_FREQ_OFFSET_RF09_RNG1_Hz, \
			      PLL_FINE_FREQ_OFFSET_RF09_RNG2_Hz}
/** RF24 frequency offset for fine mode */
#define PLL_FREQ_OFFSET_RF24 {PLL_FINE_FREQ_OFFSET_RF24_RNG3_Hz}

/** RF09 fine resolution channel modes */
#define PLL_FINE_CM_RF09     {RF215_RFn_CNM_CM_FINE_389, \
			      RF215_RFn_CNM_CM_FINE_779}
/** RF24 fine resolution channel modes */
#define PLL_FINE_CM_RF24     {RF215_RFn_CNM_CM_FINE_2400}

/** PLL constants struct initialization (RF09) */
#define PLL_CONST_RF09       {.px_freq_rng = PLL_FREQ_RNG_RF09,	\
			      .pul_fine_freq_res = PLL_FREQ_RES_RF09, \
			      .pul_fine_freq_offset = PLL_FREQ_OFFSET_RF09, \
			      .ul_ieee_freq_offset = PLL_IEEE_FREQ_OFFSET09_Hz,	\
			      .ul_fsk_tol_t0 = PLL_DELTA_FSK_T0_RF09_Q45, \
			      .puc_fine_chn_mode = PLL_FINE_CM_RF09, \
			      .uc_num_freq_rng = 2}
/** PLL constants struct initialization (RF24) */
#define PLL_CONST_RF24       {.px_freq_rng = PLL_FREQ_RNG_RF24,	\
			      .pul_fine_freq_res = PLL_FREQ_RES_RF24, \
			      .pul_fine_freq_offset = PLL_FREQ_OFFSET_RF24, \
			      .ul_ieee_freq_offset = PLL_IEEE_FREQ_OFFSET24_Hz,	\
			      .ul_fsk_tol_t0 = PLL_DELTA_FSK_T0_RF24_Q45, \
			      .puc_fine_chn_mode = PLL_FINE_CM_RF24, \
			      .uc_num_freq_rng = 1}

/** PLL constants array struct initialization */
#if defined(AT86RF215_DISABLE_RF24_TRX)
# define RF215_PLL_CONST      {PLL_CONST_RF09}
#elif defined(AT86RF215_DISABLE_RF09_TRX)
# define RF215_PLL_CONST      {PLL_CONST_RF24}
#else
# define RF215_PLL_CONST      {PLL_CONST_RF09, PLL_CONST_RF24}
#endif

/** Struct definition for PLL configuration parameters */
typedef struct pll_params {
	/* Current channel frequency in Hz */
	uint32_t ul_chn_freq;
	/* RF215 channel mode in use (CNM.CM) */
	uint8_t uc_chn_mode;
	/* RF215 frequency range in use */
	uint8_t uc_freq_rng;
	/* RF215 PLL register values */
	uint8_t puc_pll_regs[5];
} pll_params_t;

/** RF215 internal global variables declared as extern */
extern const uint16_t gpus_fsk_symrate_khz[RF215_NUM_FSK_SYMRATES];

#ifdef __cplusplus
}
#endif

#endif  /* RF215_PLL_DEFS_H_INCLUDE */
