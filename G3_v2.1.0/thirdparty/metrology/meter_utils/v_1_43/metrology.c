#include "asf.h"
#include "math.h"
#include <string.h>
#include "metrology.h"
#include "met_utils.h"
#include "shared_memory.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

METROLOGY_TYPE VMetrology;
CAL_METER_TYPE VCal_Meter;
AFE_TYPE VAFE;

const DSP_CTRL_TYPE dsp_ctrl_default = {
	{ DSP_CTRL_ST_CTRL_Run },       /* dsp_running */
	{ 0x710 },                      /* ft_ctrl0 */
	{ 7 },
	{ 0x00000CCC },                 /* sensor_type =0 CT, 1 SHUNT, 2 ROGOWSKI */

	{ 0 },                          /* M=50->50Hz M=60->60Hz */
	{ 4400 },
	{ 0x90001F40 },                 /* enable pulse,low level,width=2500=0x09C4 */
	{ 0x10001F40 },                 /* (0x010609c4), */

	{ 0x10001F40 },
	(0x500000),                     /* p_k_t=(1000/3200)*2^24=5242880   mc=3200 imp/kWh */
	(0x500000),                     /* q_k_t=(1000/3200)*2^24=5242880   mc=3200 imp/kWh */
	(0x500000),                     /* i_k_t */

	(0x10051),                      /* 5mA	creep_thr_p */
	(0x10051),                      /* 5mA	creep_thr_q */
	(0x47CB),                       /* SHUNT=150uOhm PGA =8, 15mA */
	{ 0 },                          /* power_os_ctrl */

	(0),                            /* power_os_p */
	(0),                            /* power_os_q */
	(0x5DEAD74),                    /* swell_thr_va	[(220*114%)/1651]*2^32 */
	(0x5DEAD74),                    /* swell_thr_vb */

	(0x5DEAD74),                    /* swell_thr_vc */
	(0x1A2EC26),                    /* sag_thr_va	[(220*60%)/1651]*2^32 */
	(0x1A2EC26),                    /* sag_thr_vb */
	(0x1A2EC26),                    /* sag_thr_vc */

	(0xC0E6B),                      /* k_ia	[Te=2500/3.24]*2^10 */
	(0x19CC00),                     /* k_va	1651*2^10 */
	(0xC0E6B),  /* (0x34155),        / * k_ib * / */
	(0x19CC00),                     /* k_vb */

	(0xC0E6B),  /* (0x34155),        / * k_ic * / */
	(0x19CC00),                     /* k_vc */
	(0xC0E6B),  /* (0x34155),        / * k_im * / */
	(0x20000000),                   /* cal_m_ia */

	(0x20000000),                   /* cal_m_va */
	(0x20000000),                   /* cal_m_ib */
	(0x20000000),                   /* cal_m_vb */
	(0x20000000),                   /* cal_m_ic */

	(0x20000000),                   /* cal_m_vc */
	(0x20000000),                   /* cal_m_im */
	(0),                            /* cal_ph_ia */
	(0),                            /* cal_ph_va */

	(0),                            /* cal_ph_ib */
	(0),                            /* cal_ph_vb */
	(0),                            /* cal_ph_ic */
	(0),                            /* cal_ph_vc */

	(0),                            /* reserved */
	(0),                            /* reserved */
	(0),                            /* reserved */
	(0),                            /* reserved */

	(0),                            /* reserved */
	(0),                            /* reserved */
	(0),                            /* reserved */
	{ 0x01010103 },                 /* atsense_ctrl_20_23: I2GAIN=0,I2ON=1,V1ON=1,I1GAIN=0,I1ON=1,I0GAIN=0,I0ON=1 */

	{ 0x07010101 },                 /* atsense_ctrl_24_27: ONLDO=1,ONREF=1,ONBIAS=1,V3ON=1,I3GAIN=0,I3ON=1,V2ON=1 */
	{ 0x03 },                       /* atsense_ctrl_28_2b: MSB_MODE=0,OSR=3 */
	(0)                             /* app_read_ctrl */
};

/* ------------------------------------------------------------------- */
/* ----------------------new dsp define------------------------------- */
/* ------------------------------------------------------------------- */
const uint8_t dsp_ctrl_stctrl[]         = {"00 STATE_CTRL"};            /* \r\n"}; */
const uint8_t dsp_ctrl_ftctrl0[]        = {"01 FEATURE_CTRL0"};         /* \r\n"}; */
const uint8_t dsp_ctrl_ftctrl1[]        = {"02 FEATURE_CTRL1"};         /* \r\n"}; */
const uint8_t dsp_ctrl_mt_type[]        = {"03 METER_TYPE"};            /* \r\n"}; */
const uint8_t dsp_ctrl_m[]              = {"04 M"};                     /* \r\n"}; */
const uint8_t dsp_ctrl_n_max[]          = {"05 N_MAX"};                 /* \r\n"}; */
const uint8_t dsp_ctrl_pls0_ctrl[]      = {"06 PULSE_CTRL0"};           /* \r\n"}; */
const uint8_t dsp_ctrl_pls1_ctrl[]      = {"07 PULSE_CTRL1"};           /* \r\n"}; */
const uint8_t dsp_ctrl_pls2_ctrl[]      = {"08 PULSE_CTRL2"};           /* \r\n"}; */
const uint8_t dsp_ctrl_p_k_t[]          = {"09 P_K_T"};                 /* \r\n"}; */
const uint8_t dsp_ctrl_q_k_t[]          = {"10 Q_K_T"};                 /* \r\n"}; */
const uint8_t dsp_ctrl_i_k_t[]          = {"11 I_K_T"};                 /* \r\n"}; */
const uint8_t dsp_ctrl_crp_thr_p[]      = {"12 CREEP_THR_P"};           /* \r\n"}; */
const uint8_t dsp_ctrl_crp_thr_q[]      = {"13 CREEP_THR_Q"};           /* \r\n"}; */
const uint8_t dsp_ctrl_crp_thr_i[]      = {"14 CREEP_THR_I"};           /* \r\n"}; */
const uint8_t dsp_ctrl_power_ofs_ctrl[] = {"15 POWER_OFFSET_CTL"};      /* \r\n"}; */
const uint8_t dsp_ctrl_power_ofs_p[]    = {"16 POWER_OFFSET_P"};        /* \r\n"}; */
const uint8_t dsp_ctrl_power_ofs_q[]    = {"17 POWER_OFFSET_Q"};        /* \r\n"}; */
const uint8_t dsp_ctrl_sw_thr_va[]      = {"18 SWELL_THR_VA"};          /* \r\n"}; */
const uint8_t dsp_ctrl_sw_thr_vb[]      = {"19 SWELL_THR_VB"};          /* \r\n"}; */
const uint8_t dsp_ctrl_sw_thr_vc[]      = {"20 SWELL_THR_VC"};          /* \r\n"}; */
const uint8_t dsp_ctrl_sg_thr_va[]      = {"21 SAG_THR_VA"};            /* \r\n"}; */
const uint8_t dsp_ctrl_sg_thr_vb[]      = {"22 SAG_THR_VB"};            /* \r\n"}; */
const uint8_t dsp_ctrl_sg_thr_vc[]      = {"23 SAG_THR_VC"};            /* \r\n"}; */
const uint8_t dsp_ctrl_k_ia[]           = {"24 K_IA"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_k_va[]           = {"25 K_VA"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_k_ib[]           = {"26 K_IB"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_k_vb[]           = {"27 K_VB"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_k_ic[]           = {"28 K_IC"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_k_vc[]           = {"29 K_VC"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_k_in[]           = {"30 K_IN"};                  /* \r\n"}; */
const uint8_t dsp_ctrl_cal_m_ia[]       = {"31 CAL_M_IA"};              /* \r\n"}; */
const uint8_t dsp_ctrl_cal_m_va[]       = {"32 CAL_M_VA"};              /* \r\n"}; */
const uint8_t dsp_ctrl_cal_m_ib[]       = {"33 CAL_M_IB"};              /* \r\n"}; */
const uint8_t dsp_ctrl_cal_m_vb[]       = {"34 CAL_M_VB"};              /* \r\n"}; */
const uint8_t dsp_ctrl_cal_m_ic[]       = {"35 CAL_M_IC"};              /* \r\n"}; */
const uint8_t dsp_ctrl_cal_m_vc[]       = {"36 CAL_M_VC"};              /* \r\n"}; */
const uint8_t dsp_ctrl_reserved2[]      = {"37 RESERVED"};              /* \r\n"}; */
const uint8_t dsp_ctrl_cal_ph_ia[]      = {"38 CAL_PH_IA"};             /* \r\n"}; */
const uint8_t dsp_ctrl_cal_ph_va[]      = {"39 CAL_PH_VA"};             /* \r\n"}; */
const uint8_t dsp_ctrl_cal_ph_ib[]      = {"40 CAL_PH_IB"};             /* \r\n"}; */
const uint8_t dsp_ctrl_cal_ph_vb[]      = {"41 CAL_PH_VB"};             /* \r\n"}; */
const uint8_t dsp_ctrl_cal_ph_ic[]      = {"42 CAL_PH_IC"};             /* \r\n"}; */
const uint8_t dsp_ctrl_cal_ph_vc[]      = {"43 CAL_PH_VC"};             /* \r\n"}; */
const uint8_t dsp_ctrl_reserved3[]      = {"44 RESERVED"};              /* \r\n"}; */
const uint8_t dsp_ctrl_reserved4[]      = {"45 CAPT_CTRL"};             /* \r\n"}; */
const uint8_t dsp_ctrl_reserved5[]      = {"46 CAPT_BUFF_SIZE"};        /* \r\n"}; */
const uint8_t dsp_ctrl_reserved6[]      = {"47 CAPT_ADDR"};             /* \r\n"}; */
const uint8_t dsp_ctrl_reserved7[]      = {"48 RESERVED"};              /* \r\n"}; */
const uint8_t dsp_ctrl_reserved8[]      = {"49 RESERVED"};              /* \r\n"}; */
const uint8_t dsp_ctrl_reserved9[]      = {"50 RESERVED"};              /* \r\n"}; */
const uint8_t dsp_ctrl_at_ctrl_20_23[]  = {"51 AT_CTRL_20_23"};         /* \r\n"}; */
const uint8_t dsp_ctrl_at_ctrl_24_27[]  = {"52 AT_CTRL_24_27"};         /* \r\n"}; */
const uint8_t dsp_ctrl_at_ctrl_28_2b[]  = {"53 AT_CTRL_28_2B"};         /* \r\n"}; */
const uint8_t dsp_ctrl_reserved10[]     = {"54 RESERVED"};              /* \r\n"}; */

const uint8_t *dsp_ctrl_header[] = {
	dsp_ctrl_stctrl,
	dsp_ctrl_ftctrl0,
	dsp_ctrl_ftctrl1,
	dsp_ctrl_mt_type,
	dsp_ctrl_m,
	dsp_ctrl_n_max,
	dsp_ctrl_pls0_ctrl,
	dsp_ctrl_pls1_ctrl,
	dsp_ctrl_pls2_ctrl,
	dsp_ctrl_p_k_t,
	dsp_ctrl_q_k_t,
	dsp_ctrl_i_k_t,
	dsp_ctrl_crp_thr_p,
	dsp_ctrl_crp_thr_q,
	dsp_ctrl_crp_thr_i,
	dsp_ctrl_power_ofs_ctrl,
	dsp_ctrl_power_ofs_p,
	dsp_ctrl_power_ofs_q,
	dsp_ctrl_sw_thr_va,
	dsp_ctrl_sw_thr_vb,
	dsp_ctrl_sw_thr_vc,
	dsp_ctrl_sg_thr_va,
	dsp_ctrl_sg_thr_vb,
	dsp_ctrl_sg_thr_vc,
	dsp_ctrl_k_ia,
	dsp_ctrl_k_va,
	dsp_ctrl_k_ib,
	dsp_ctrl_k_vb,
	dsp_ctrl_k_ic,
	dsp_ctrl_k_vc,
	dsp_ctrl_k_in,
	dsp_ctrl_cal_m_ia,
	dsp_ctrl_cal_m_va,
	dsp_ctrl_cal_m_ib,
	dsp_ctrl_cal_m_vb,
	dsp_ctrl_cal_m_ic,
	dsp_ctrl_cal_m_vc,
	dsp_ctrl_reserved2,
	dsp_ctrl_cal_ph_ia,
	dsp_ctrl_cal_ph_va,
	dsp_ctrl_cal_ph_ib,
	dsp_ctrl_cal_ph_vb,
	dsp_ctrl_cal_ph_ic,
	dsp_ctrl_cal_ph_vc,
	dsp_ctrl_reserved3,
	dsp_ctrl_reserved4,
	dsp_ctrl_reserved5,
	dsp_ctrl_reserved6,
	dsp_ctrl_reserved7,
	dsp_ctrl_reserved8,
	dsp_ctrl_reserved9,
	dsp_ctrl_at_ctrl_20_23,
	dsp_ctrl_at_ctrl_24_27,
	dsp_ctrl_at_ctrl_28_2b,
	dsp_ctrl_reserved10,
	NULL
};

const uint32_t *dsp_ctrl_str[] = {
	&VMetrology.DSP_CTRL.STATE_CTRL.WORD,
	&VMetrology.DSP_CTRL.FEATURE_CTRL0.WORD,
	&VMetrology.DSP_CTRL.FEATURE_CTRL1.WORD,
	&VMetrology.DSP_CTRL.METER_TYPE.WORD,
	&VMetrology.DSP_CTRL.M.WORD,
	&VMetrology.DSP_CTRL.N_MAX.WORD,
	&VMetrology.DSP_CTRL.PULSE0_CTRL.WORD,
	&VMetrology.DSP_CTRL.PULSE1_CTRL.WORD,
	&VMetrology.DSP_CTRL.PULSE2_CTRL.WORD,
	&VMetrology.DSP_CTRL.P_K_T,
	&VMetrology.DSP_CTRL.Q_K_T,
	&VMetrology.DSP_CTRL.I_K_T,
	&VMetrology.DSP_CTRL.CREEP_THR_P,
	&VMetrology.DSP_CTRL.CREEP_THR_Q,
	&VMetrology.DSP_CTRL.CREEP_THR_I,
	&VMetrology.DSP_CTRL.POWER_OFFSET_CTRL.WORD,
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_P),
	(uint32_t *)(&VMetrology.DSP_CTRL.POWER_OFFSET_Q),
	&VMetrology.DSP_CTRL.SWELL_THR_VA,
	&VMetrology.DSP_CTRL.SWELL_THR_VB,
	&VMetrology.DSP_CTRL.SWELL_THR_VC,
	&VMetrology.DSP_CTRL.SAG_THR_VA,
	&VMetrology.DSP_CTRL.SAG_THR_VB,
	&VMetrology.DSP_CTRL.SAG_THR_VC,
	&VMetrology.DSP_CTRL.K_IA,
	&VMetrology.DSP_CTRL.K_VA,
	&VMetrology.DSP_CTRL.K_IB,
	&VMetrology.DSP_CTRL.K_VB,
	&VMetrology.DSP_CTRL.K_IC,
	&VMetrology.DSP_CTRL.K_VC,
	&VMetrology.DSP_CTRL.K_IN,
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_VA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_VB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_IC),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_M_VC),
	(uint32_t *)(&VMetrology.DSP_CTRL.RESERVED2),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_VA),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_VB),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_IC),
	(uint32_t *)(&VMetrology.DSP_CTRL.CAL_PH_VC),
	(uint32_t *)(&VMetrology.DSP_CTRL.RESERVED3),
	&VMetrology.DSP_CTRL.RESERVED4,
	&VMetrology.DSP_CTRL.RESERVED5,
	&VMetrology.DSP_CTRL.RESERVED6,
	&VMetrology.DSP_CTRL.RESERVED7,
	&VMetrology.DSP_CTRL.RESERVED8,
	&VMetrology.DSP_CTRL.RESERVED9,
	&VMetrology.DSP_CTRL.ATsense_CTRL_20_23.WORD,
	&VMetrology.DSP_CTRL.ATsense_CTRL_24_27.WORD,
	&VMetrology.DSP_CTRL.ATsense_CTRL_28_2B.WORD,
	&VMetrology.DSP_CTRL.RESERVED10,
	NULL
};

const uint8_t dsp_st_version[]           = {"00 VERSION"};              /* \r\n"}; */
const uint8_t dsp_st_status[]            = {"01 STATUS"};               /* \r\n"}; */
const uint8_t dsp_st_st_flag[]           = {"02 STATE_FLAG"};           /* \r\n"}; */
const uint8_t dsp_st_cp_st[]             = {"03 CAPTURE_STATUS"};       /* \r\n"}; */
const uint8_t dsp_st_interal_num[]       = {"04 INTERVAL_NUM"};         /* \r\n"}; */
const uint8_t dsp_st_n[]                 = {"05 N"};                    /* \r\n"}; */
const uint8_t dsp_st_ph_offset[]         = {"06 PH_OFFSET"};            /* \r\n"}; */
const uint8_t dsp_st_freq[]              = {"07 RREQ"};                 /* \r\n"}; */
const uint8_t dsp_st_freq_va[]           = {"08 FREQ_VA"};              /* \r\n"}; */
const uint8_t dsp_st_freq_vb[]           = {"09 FREQ_VB"};              /* \r\n"}; */
const uint8_t dsp_st_freq_vc[]           = {"10 FREQ_VC"};              /* \r\n"}; */
const uint8_t dsp_st_reserved0[]         = {"11 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_temperature[]       = {"12 TEMPERATURE"};          /* \r\n"}; */
const uint8_t dsp_st_i_a_max[]           = {"13 I_A_MAX"};              /* \r\n"}; */
const uint8_t dsp_st_i_b_max[]           = {"14 I_B_MAX"};              /* \r\n"}; */
const uint8_t dsp_st_i_c_max[]           = {"15 I_C_MAX"};              /* \r\n"}; */
const uint8_t dsp_st_i_ni_max[]          = {"16 I_Ni_MAX"};             /* \r\n"}; */
const uint8_t dsp_st_i_nm_max[]          = {"17 I_Nm_MAX"};             /* \r\n"}; */
const uint8_t dsp_st_v_a_max[]           = {"18 V_A_MAX"};              /* \r\n"}; */
const uint8_t dsp_st_v_b_max[]           = {"19 V_B_MAX"};              /* \r\n"}; */
const uint8_t dsp_st_v_c_max[]           = {"20 V_C_MAX"};              /* \r\n"}; */
const uint8_t dsp_st_reserved1[]         = {"21 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved2[]         = {"22 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved3[]         = {"23 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved4[]         = {"24 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved5[]         = {"25 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved6[]         = {"26 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved7[]         = {"27 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved8[]         = {"28 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_reserved9[]         = {"29 RESERVED"};             /* \r\n"}; */
const uint8_t dsp_st_zc_n_va[]           = {"30 ZC_N_VA"};              /* \r\n"}; */
const uint8_t dsp_st_zc_n_vb[]           = {"31 ZC_N_VB"};              /* \r\n"}; */
const uint8_t dsp_st_zc_n_vc[]           = {"32 ZC_N_VC"};              /* \r\n"}; */
const uint8_t dsp_st_atsense_cal_41_44[] = {"33 AT_CAL_41_44"};         /* \r\n"}; */
const uint8_t dsp_st_atsense_cal_45_48[] = {"34 AT_CAL_45_48"};         /* \r\n"}; */

const uint8_t *dsp_st_header[] = {
	dsp_st_version,
	dsp_st_status,
	dsp_st_st_flag,
	dsp_st_cp_st,
	dsp_st_interal_num,
	dsp_st_n,
	dsp_st_ph_offset,
	dsp_st_freq,
	dsp_st_freq_va,
	dsp_st_freq_vb,
	dsp_st_freq_vc,
	dsp_st_reserved0,
	dsp_st_temperature,
	dsp_st_i_a_max,
	dsp_st_i_b_max,
	dsp_st_i_c_max,
	dsp_st_i_ni_max,
	dsp_st_i_nm_max,
	dsp_st_v_a_max,
	dsp_st_v_b_max,
	dsp_st_v_c_max,
	dsp_st_reserved1,
	dsp_st_reserved2,
	dsp_st_reserved3,
	dsp_st_reserved4,
	dsp_st_reserved5,
	dsp_st_reserved6,
	dsp_st_reserved7,
	dsp_st_reserved8,
	dsp_st_reserved9,
	dsp_st_zc_n_va,
	dsp_st_zc_n_vb,
	dsp_st_zc_n_vc,
	dsp_st_atsense_cal_41_44,
	dsp_st_atsense_cal_45_48,
	NULL
};

const uint32_t *dsp_st_str[] = {
	&VMetrology.DSP_ST.VERSION.WORD,
	&VMetrology.DSP_ST.STATUS.WORD,
	&VMetrology.DSP_ST.STATE_FLAG.WORD,
	&VMetrology.DSP_ST.CP_ST,
	&VMetrology.DSP_ST.INTERVAL_NUM,
	&VMetrology.DSP_ST.N,
	(uint32_t *)(&VMetrology.DSP_ST.PH_OFFSET),
	&VMetrology.DSP_ST.FREQ,
	&VMetrology.DSP_ST.FREQ_VA,
	&VMetrology.DSP_ST.FREQ_VB,
	&VMetrology.DSP_ST.FREQ_VC,
	&VMetrology.DSP_ST.RESERVED0,
	(uint32_t *)(&VMetrology.DSP_ST.TEMPERATURE),
	(uint32_t *)(&VMetrology.DSP_ST.I_A_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.I_B_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.I_C_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.I_Ni_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.I_Nm_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.V_A_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.V_B_MAX),
	(uint32_t *)(&VMetrology.DSP_ST.V_C_MAX),
	&VMetrology.DSP_ST.RESERVED1,
	&VMetrology.DSP_ST.RESERVED2,
	&VMetrology.DSP_ST.RESERVED3,
	&VMetrology.DSP_ST.RESERVED4,
	&VMetrology.DSP_ST.RESERVED5,
	&VMetrology.DSP_ST.RESERVED6,
	&VMetrology.DSP_ST.RESERVED7,
	&VMetrology.DSP_ST.RESERVED8,
	&VMetrology.DSP_ST.RESERVED9,
	&VMetrology.DSP_ST.ZC_N_VA,
	&VMetrology.DSP_ST.ZC_N_VB,
	&VMetrology.DSP_ST.ZC_N_VC,
	&VMetrology.DSP_ST.ATsense_CAL_41_44.WORD,
	&VMetrology.DSP_ST.ATsense_CAL_45_48.WORD,
	NULL
};

const uint8_t dsp_acc_i_a[]        = {"00 I_A"};                /* \r\n"}; */
const uint8_t dsp_acc_i_b[]        = {"01 I_B"};                /* \r\n"}; */
const uint8_t dsp_acc_i_c[]        = {"02 I_C"};                /* \r\n"}; */
const uint8_t dsp_acc_i_ni[]       = {"03 I_Ni"};               /* \r\n"}; */
const uint8_t dsp_acc_i_nm[]       = {"04 I_Nm"};               /* \r\n"}; */
const uint8_t dsp_acc_i_a_f[]      = {"05 I_A_F"};              /* \r\n"}; */
const uint8_t dsp_acc_i_b_f[]      = {"06 I_B_F"};              /* \r\n"}; */
const uint8_t dsp_acc_i_c_f[]      = {"07 I_C_F"};              /* \r\n"}; */
const uint8_t dsp_acc_i_ni_f[]     = {"08 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved0[]  = {"09 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved1[]  = {"10 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved2[]  = {"11 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved3[]  = {"12 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved4[]  = {"13 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved5[]  = {"14 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_p_a[]        = {"15 P_A"};                /* \r\n"}; */
const uint8_t dsp_acc_p_b[]        = {"16 P_B"};                /* \r\n"}; */
const uint8_t dsp_acc_p_c[]        = {"17 P_C"};                /* \r\n"}; */
const uint8_t dsp_acc_p_a_f[]      = {"18 P_A_F"};              /* \r\n"}; */
const uint8_t dsp_acc_p_b_f[]      = {"19 P_B_F"};              /* \r\n"}; */
const uint8_t dsp_acc_p_c_f[]      = {"20 P_C_F"};              /* \r\n"}; */
const uint8_t dsp_acc_reserved6[]  = {"21 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved7[]  = {"22 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved8[]  = {"23 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_q_a[]        = {"24 Q_A"};                /* \r\n"}; */
const uint8_t dsp_acc_q_b[]        = {"25 Q_B"};                /* \r\n"}; */
const uint8_t dsp_acc_q_c[]        = {"26 Q_C"};                /* \r\n"}; */
const uint8_t dsp_acc_q_a_f[]      = {"27 Q_A_F"};              /* \r\n"}; */
const uint8_t dsp_acc_q_b_f[]      = {"28 Q_B_F"};              /* \r\n"}; */
const uint8_t dsp_acc_q_c_f[]      = {"29 Q_C_F"};              /* \r\n"}; */
const uint8_t dsp_acc_reserved9[]  = {"30 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved10[] = {"31 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved11[] = {"32 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_v_a[]        = {"33 V_A"};                /* \r\n"}; */
const uint8_t dsp_acc_v_b[]        = {"34 V_B"};                /* \r\n"}; */
const uint8_t dsp_acc_v_c[]        = {"35 V_C"};                /* \r\n"}; */
const uint8_t dsp_acc_reserved12[] = {"36 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_v_a_f[]      = {"37 V_A_F"};              /* \r\n"}; */
const uint8_t dsp_acc_v_b_f[]      = {"38 V_B_F"};              /* \r\n"}; */
const uint8_t dsp_acc_v_c_f[]      = {"39 V_C_F"};              /* \r\n"}; */
const uint8_t dsp_acc_reserved13[] = {"40 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_v_ab[]       = {"41 V_AB"};               /* \r\n"}; */
const uint8_t dsp_acc_v_bc[]       = {"42 V_BC"};               /* \r\n"}; */
const uint8_t dsp_acc_v_ca[]       = {"43 V_CA"};               /* \r\n"}; */
const uint8_t dsp_acc_v_ab_f[]     = {"44 V_AB_F"};             /* \r\n"}; */
const uint8_t dsp_acc_v_bc_f[]     = {"45 V_BC_F"};             /* \r\n"}; */
const uint8_t dsp_acc_v_ca_f[]     = {"46 V_CA_F"};             /* \r\n"}; */
const uint8_t dsp_acc_reserved14[] = {"47 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved15[] = {"48 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved16[] = {"49 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_acc_t0[]     = {"50 ACC_T0"};             /* \r\n"}; */
const uint8_t dsp_acc_acc_t1[]     = {"51 ACC_T1"};             /* \r\n"}; */
const uint8_t dsp_acc_acc_t2[]     = {"52 ACC_T2"};             /* \r\n"}; */
const uint8_t dsp_acc_reserved17[] = {"53 RESERVED"};           /* \r\n"}; */
const uint8_t dsp_acc_reserved18[] = {"54 RESERVED"};           /* \r\n"}; */

const uint8_t *dsp_acc_header[] = {
	dsp_acc_i_a,
	dsp_acc_i_b,
	dsp_acc_i_c,
	dsp_acc_i_ni,
	dsp_acc_i_nm,
	dsp_acc_i_a_f,
	dsp_acc_i_b_f,
	dsp_acc_i_c_f,
	dsp_acc_i_ni_f,
	dsp_acc_reserved0,
	dsp_acc_reserved1,
	dsp_acc_reserved2,
	dsp_acc_reserved3,
	dsp_acc_reserved4,
	dsp_acc_reserved5,
	dsp_acc_p_a,
	dsp_acc_p_b,
	dsp_acc_p_c,
	dsp_acc_p_a_f,
	dsp_acc_p_b_f,
	dsp_acc_p_c_f,
	dsp_acc_reserved6,
	dsp_acc_reserved7,
	dsp_acc_reserved8,
	dsp_acc_q_a,
	dsp_acc_q_b,
	dsp_acc_q_c,
	dsp_acc_q_a_f,
	dsp_acc_q_b_f,
	dsp_acc_q_c_f,
	dsp_acc_reserved9,
	dsp_acc_reserved10,
	dsp_acc_reserved11,
	dsp_acc_v_a,
	dsp_acc_v_b,
	dsp_acc_v_c,
	dsp_acc_reserved12,
	dsp_acc_v_a_f,
	dsp_acc_v_b_f,
	dsp_acc_v_c_f,
	dsp_acc_reserved13,
	dsp_acc_v_ab,
	dsp_acc_v_bc,
	dsp_acc_v_ca,
	dsp_acc_v_ab_f,
	dsp_acc_v_bc_f,
	dsp_acc_v_ca_f,
	dsp_acc_reserved14,
	dsp_acc_reserved15,
	dsp_acc_reserved16,
	dsp_acc_acc_t0,
	dsp_acc_acc_t1,
	dsp_acc_acc_t2,
	dsp_acc_reserved17,
	dsp_acc_reserved18,
	NULL
};

const uint64_t *dsp_acc_str[] = {
	&VMetrology.DSP_ACC.I_A,
	&VMetrology.DSP_ACC.I_B,
	&VMetrology.DSP_ACC.I_C,
	&VMetrology.DSP_ACC.I_Ni,
	&VMetrology.DSP_ACC.I_Nm,
	&VMetrology.DSP_ACC.I_A_F,
	&VMetrology.DSP_ACC.I_B_F,
	&VMetrology.DSP_ACC.I_C_F,
	&VMetrology.DSP_ACC.I_Ni_F,
	&VMetrology.DSP_ACC.RESERVED0,
	&VMetrology.DSP_ACC.RESERVED1,
	&VMetrology.DSP_ACC.RESERVED2,
	&VMetrology.DSP_ACC.RESERVED3,
	&VMetrology.DSP_ACC.RESERVED4,
	&VMetrology.DSP_ACC.RESERVED5,
	(uint64_t *)(&VMetrology.DSP_ACC.P_A),
	(uint64_t *)(&VMetrology.DSP_ACC.P_B),
	(uint64_t *)(&VMetrology.DSP_ACC.P_C),
	(uint64_t *)(&VMetrology.DSP_ACC.P_A_F),
	(uint64_t *)(&VMetrology.DSP_ACC.P_B_F),
	(uint64_t *)(&VMetrology.DSP_ACC.P_C_F),
	&VMetrology.DSP_ACC.RESERVED6,
	&VMetrology.DSP_ACC.RESERVED7,
	&VMetrology.DSP_ACC.RESERVED8,
	(uint64_t *)(&VMetrology.DSP_ACC.Q_A),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_B),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_C),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_A_F),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_B_F),
	(uint64_t *)(&VMetrology.DSP_ACC.Q_C_F),
	&VMetrology.DSP_ACC.RESERVED9,
	&VMetrology.DSP_ACC.RESERVED10,
	&VMetrology.DSP_ACC.RESERVED11,
	&VMetrology.DSP_ACC.V_A,
	&VMetrology.DSP_ACC.V_B,
	&VMetrology.DSP_ACC.V_C,
	&VMetrology.DSP_ACC.RESERVED12,
	&VMetrology.DSP_ACC.V_A_F,
	&VMetrology.DSP_ACC.V_B_F,
	&VMetrology.DSP_ACC.V_C_F,
	&VMetrology.DSP_ACC.RESERVED13,
	&VMetrology.DSP_ACC.V_AB,
	&VMetrology.DSP_ACC.V_BC,
	&VMetrology.DSP_ACC.V_CA,
	&VMetrology.DSP_ACC.V_AB_F,
	&VMetrology.DSP_ACC.V_BC_F,
	&VMetrology.DSP_ACC.V_CA_F,
	&VMetrology.DSP_ACC.RESERVED14,
	&VMetrology.DSP_ACC.RESERVED15,
	&VMetrology.DSP_ACC.RESERVED16,
	&VMetrology.DSP_ACC.ACC_T0,
	&VMetrology.DSP_ACC.ACC_T1,
	&VMetrology.DSP_ACC.ACC_T2,
	&VMetrology.DSP_ACC.RESERVED17,
	&VMetrology.DSP_ACC.RESERVED18,
	NULL
};

const uint8_t dsp_harmonic_i_a_r[]      = {"00 I_A_R"};         /* \r\n"}; */
const uint8_t dsp_harmonic_v_a_r[]      = {"01 V_A_R"};         /* \r\n"}; */
const uint8_t dsp_harmonic_i_b_r[]      = {"02 I_B_R"};         /* \r\n"}; */
const uint8_t dsp_harmonic_v_b_r[]      = {"03 V_B_R"};         /* \r\n"}; */
const uint8_t dsp_harmonic_i_c_r[]      = {"04 I_C_R"};         /* \r\n"}; */
const uint8_t dsp_harmonic_v_c_r[]      = {"05 V_C_R"};         /* \r\n"}; */
const uint8_t dsp_harmonic_i_a_i[]      = {"06 I_A_I"};         /* \r\n"}; */
const uint8_t dsp_harmonic_v_a_i[]      = {"07 V_A_I"};         /* \r\n"}; */
const uint8_t dsp_harmonic_i_b_i[]      = {"08 I_B_I"};         /* \r\n"}; */
const uint8_t dsp_harmonic_v_b_i[]      = {"09 V_B_I"};         /* \r\n"}; */
const uint8_t dsp_harmonic_i_c_i[]      = {"10 I_C_I"};         /* \r\n"}; */
const uint8_t dsp_harmonic_v_c_i[]      = {"11 V_C_I"};         /* \r\n"}; */
const uint8_t dsp_harmonic_reserved0[]  = {"12 RESERVED"};      /* \r\n"}; */
const uint8_t dsp_harmonic_reserved1[]  = {"13 RESERVED"};      /* \r\n"}; */
const uint8_t dsp_harmonic_reserved2[]  = {"14 RESERVED"};      /* \r\n"}; */

const uint8_t *dsp_har_header[] = {
	dsp_harmonic_i_a_r,
	dsp_harmonic_v_a_r,
	dsp_harmonic_i_b_r,
	dsp_harmonic_v_b_r,
	dsp_harmonic_i_c_r,
	dsp_harmonic_v_c_r,
	dsp_harmonic_i_a_i,
	dsp_harmonic_v_a_i,
	dsp_harmonic_i_b_i,
	dsp_harmonic_v_b_i,
	dsp_harmonic_i_c_i,
	dsp_harmonic_v_c_i,
	dsp_harmonic_reserved0,
	dsp_harmonic_reserved1,
	dsp_harmonic_reserved2,
	NULL
};

const uint32_t *dsp_har_str[] = {
	&VMetrology.DSP_HAR.I_A_R,
	&VMetrology.DSP_HAR.V_A_R,
	&VMetrology.DSP_HAR.I_B_R,
	&VMetrology.DSP_HAR.V_B_R,
	&VMetrology.DSP_HAR.I_C_R,
	&VMetrology.DSP_HAR.V_C_R,
	&VMetrology.DSP_HAR.I_A_I,
	&VMetrology.DSP_HAR.V_A_I,
	&VMetrology.DSP_HAR.I_B_I,
	&VMetrology.DSP_HAR.V_B_I,
	&VMetrology.DSP_HAR.I_C_I,
	&VMetrology.DSP_HAR.V_C_I,
	&VMetrology.DSP_HAR.RESERVED0,
	&VMetrology.DSP_HAR.RESERVED1,
	&VMetrology.DSP_HAR.RESERVED2,
	NULL
};
/* ------------------------------------------------------------------- */

/* =================================================================== */
/* description	::	initialize dsp */
/* function		::	m_dsp_init */
/* input		::	none */
/* output		::	none */
/* call			::	none */
/* effect		::	dsp */
/* =================================================================== */
void m_dsp_init( void )
{
	/* make sure the the DSP data structures are agreed between DSP and Meter */
	Assert( DSP_CONTROL_SIZE == METROLOGY_REG_IN_SIZE );
	Assert( DSP_ST_SIZE == METROLOGY_REG_OUT_SIZE );
	Assert( DSP_ACC_SIZE == METROLOGY_ACC_OUT_SIZE );

	dsp_init();
}

/* =================================================================== */
/* description	::	set dsp ctrl st ctrl status */
/* function		::	Set_DSP_CTRL_ST_CTRL */
/* input		::	iSStatus */
/* output		::	none */
/* call			::	none */
/* effect		::	VMetrology.DSP_CTRL mem_reg_in */
/* =================================================================== */
void Set_DSP_CTRL_ST_CTRL( DSP_CTRL_ST_CTRL_TYPE iSStatus )
{
	VMetrology.DSP_CTRL.STATE_CTRL.BIT.ST_CTRL = iSStatus;
	((DSP_CTRL_TYPE *)(mem_reg_in))->STATE_CTRL.BIT.ST_CTRL = iSStatus;
}

/* =================================================================== */
/* description	::	get dsp run status */
/* function		::	Get_DSP_Status */
/* input		::	none */
/* output		::	dsp_status */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t Get_DSP_Status( void )
{
	return (VMetrology.DSP_ST.STATUS.BIT.ST);
}

/* =================================================================== */
/* description	::	initialize dsp */
/* function		::	dsp_init */
/* input		::	none */
/* output		::	1=success, 0=failure */
/* call			::	none */
/* effect		::	dsp */
/*	DSP on Reset state at power up. Steps for starting the Metrology DSP should be: */
/*	(1) change CONTROL command state to RESET; */
/*	(2) wait for DSP to complete RESET; */
/*	(3) write control registers; */
/*	(4) change CONTROL command state to INIT; */
/*	(5) wait for DSP to complete INIT; */
/*	(6) change CONTROL command to RUN; */
/*	(7) wait for DSP is RUNNING. */
/* =================================================================== */
uint32_t dsp_init( void )
{
	uint32_t i;
	uint32_t status;

	status = Get_DSP_Status();

	/* ----1---set DSP_CTRL structure---except ST_CTRL-------- */
	Set_DSP_CTRL_ST_CTRL( IsReset );

	/* ----2---wait for DSP Reset------------------------------ */
	for (i = 0; i < 200; i++) {
		if (((DSP_ST_TYPE *)(mem_reg_out))->STATUS.BIT.ST == DSP_ST_Reset) {
			break;
		}
	}

	status = Get_DSP_Status();

	/* ----3---set Metrolgoy Configure structure-------------- */
	for (i = 1; i < (DSP_CONTROL_SIZE); i++) {
		*((uint32_t *)(mem_reg_in) + i) = *((&VMetrology.DSP_CTRL.STATE_CTRL.WORD) + i);
	}

	/* Configure waveform capture buffer length */
	VMetrology.DSP_CTRL.RESERVED5   = 0;
	/* Configure waveform capture buffer address */
	VMetrology.DSP_CTRL.RESERVED6   = 0;

	/* ----4---set DSP_CTRL_ST_CTRL =Init--------------------- */
	Set_DSP_CTRL_ST_CTRL( IsInit );

	/* ----5---wait for DSP Ready------------------------------ */
	for (i = 0; i < 200; i++) {
		if (((DSP_ST_TYPE *)(mem_reg_out))->STATUS.BIT.ST == DSP_ST_Ready) {
			break;
		}
	}

	status = Get_DSP_Status();

	/* ----6---set DSP_CTRL_ST_CTRL =Run---------------------- */
	Set_DSP_CTRL_ST_CTRL( IsRun );

	/* ----7---wait for DSP Running---------------------------- */
	for (i = 0; i < 200; i++) {
		if (((DSP_ST_TYPE *)(mem_reg_out))->STATUS.BIT.ST == DSP_ST_DSP_Running) {
			break;
		}
	}

	status = Get_DSP_Status();

	((DSP_ACC_TYPE *)(mem_acc_out))->ACC_T0 = VMetrology.DSP_ACC.ACC_T0;
	((DSP_ACC_TYPE *)(mem_acc_out))->ACC_T1 = VMetrology.DSP_ACC.ACC_T1;
	((DSP_ACC_TYPE *)(mem_acc_out))->ACC_T2 = VMetrology.DSP_ACC.ACC_T2;

	return (1);
}

/* =================================================================== */
/* description	::	dsp load default */
/* function		::	DSP_Load_Default */
/* input		::	none */
/* output		::	none */
/* call			::	none */
/* effect		::	VMetrology.DSP_CTRL */
/* =================================================================== */
void DSP_Load_Default( void )
{
	uint32_t i;

	for (i = 0; i < DSP_CONTROL_SIZE; i++) {
		*((uint32_t *)(&VMetrology.DSP_CTRL.STATE_CTRL.WORD) + i) = *((uint32_t *)(&dsp_ctrl_default) + i);
	}

	VMetrology.DSP_ACC.ACC_T0 = 0;
	VMetrology.DSP_ACC.ACC_T1 = 0;
	VMetrology.DSP_ACC.ACC_T1 = 0;
}

/* =================================================================== */
/* description	::	calculate voltage or current rms value */
/* function		::	calculate_V_rms */
/* input		::	val,k_x */
/* output		::	calculation value *1000 (0.001V / 0.001A) (hex) */
/* call			::	sqrt */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_VI_rms( uint64_t val, uint32_t k_x )
{
	double m;
	m = (double)(val);
	m = (m / RMS_DIV_Q);                    /* m =m/2^40 */
	m = (m / VMetrology.DSP_ST.N);
	m = sqrt( m );                          /* m =sqrt(m) */
	m = m * k_x / RMS_DIV_G;                /* m =m*k_x */
	m = m * 1000;                           /* m =m*1000 */
	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate voltage or current rms value to bcd */
/* function		::	calculate_VI_rms_bcd */
/* input		::	val,k_x */
/* output		::	calculation value *1000 (0.001V / 0.001A) (bcd) */
/* call			::	calculate_VI_rms,met_utils_hex_4byte_to_bcd */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_VI_rms_bcd( uint64_t val, uint32_t k_x )
{
	uint32_t i;
	i = calculate_VI_rms( val, k_x );
	i = met_utils_hex_4byte_to_bcd( i );
	return (i);
}

/* =================================================================== */
/* description	::	calculate angle rms value */
/* function		::	calculate_Angle_rms */
/* input		::	p,q */
/* output		::	calculation value *100  (angle value) xx.xx  (hex) */
/* call			::	atan */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_Angle_rms( int64_t p, int64_t q )
{
	float m, n;
	if (p < 0) {
		/*		k =1; */
		m = (float)((~p) + 1);
	} else {
		m = (float)(p);
	}

	if (q < 0) {
		/*		t =1; */
		n = (float)((~q) + 1);
	} else {
		n = (float)(q);
	}

	m = 180 * atan( n / m );
	m = m * 100;                            /* *100 */
	m = m / CONST_Pi;                       /*  */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate active or reactive power rms value */
/* function		::	calculate_PQ_rms */
/* input		::	val,k_ix,k_vx */
/* output		::	calculation value *10	 (0.1w)  (hex) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_PQ_rms( int64_t val, uint32_t k_ix, uint32_t k_vx )
{
	double m;
	if (val >= (RMS_PQ_SYMB)) {
		m = (double)((~val) + 1);
	} else {
		m = (double)(val);
	}

	m = (m / RMS_DIV_Q);                            /* m =m/2^40 */
	/* m = m / 4000;                                / * m =m/4000 * / */
	m = (m / VMetrology.DSP_ST.N);
	m = (m * k_ix * k_vx) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
	m = m * 10;                                     /* m =m*10 (0.1w) */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate active or reactive power rms value to bcd */
/* function		::	calculate_PQ_rms_bcd */
/* input		::	val,k_ix,k_vx */
/* output		::	calculation value *10	 (0.1w)  (bcd) */
/* call			::	calculate_PQ_rms,met_utils_hex_4byte_to_bcd */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_PQ_rms_bcd( int64_t val, uint32_t k_ix, uint32_t k_vx )
{
	uint32_t i;
	i = calculate_PQ_rms( val, k_ix, k_vx );
	i = met_utils_hex_4byte_to_bcd( i );
	return (i);
}

/* =================================================================== */
/* description	::	calculate apparent power rms value */
/* function		::	calculate_S_rms */
/* input		::	p,q */
/* output		::	calculation value *10	 (0.1w) (hex) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_S_rms( int64_t pv, int64_t qv, uint32_t k_ix, uint32_t k_vx )
{
	double m, n;
	if (pv >= (RMS_PQ_SYMB)) {
		m = (double)((~pv) + 1);
	} else {
		m = (double)(pv);
	}

	m = (m / RMS_DIV_Q);                            /* m =m/2^40 */
	/* m = m / 4000;                                / * m =m/4000 * / */
	m = (m / VMetrology.DSP_ST.N);
	m = (m * k_ix * k_vx) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
	m = m * 10;                                     /* m =m*10 (0.1w) */

	if (qv >= (RMS_PQ_SYMB)) {
		n = (double)((~qv) + 1);
	} else {
		n = (double)(qv);
	}

	n = (n / RMS_DIV_Q);                            /* m =m/2^40 */
	/* n = n / 4000;                                / * n =n/4000 * / */
	n = (n / VMetrology.DSP_ST.N);
	n = (n * k_ix * k_vx) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
	n = n * 10;                                     /* m =m*10 (0.1w) */

	m = m * m;                                      /* *100 */
	n = n * n;                                      /* *100 */
	m = sqrt( m + n );                              /* s=[ 100*( (p^2)+(q^2) ) ]^0.5 */

	return ((uint32_t)(m));
}

/* =================================================================== */
/* description	::	calculate apparent power rms value change to bcd */
/* function		::	calculate_S_rms_bcd */
/* input		::	p,q */
/* output		::	calculation value *10	 (0.1w) (bcd) */
/* call			::	calculate_S_rms,met_utils_hex_4byte_to_bcd */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_S_rms_bcd( int64_t pv, int64_t qv, uint32_t k_ix, uint32_t k_vx )
{
	uint32_t i;
	i = calculate_S_rms( pv, qv, k_ix, k_vx );
	i = met_utils_hex_4byte_to_bcd( i );
	return (i);
}

/* =================================================================== */
/* description	::	calculate active/reactive/apparent total power rms value */
/* function		::	calculate_PQSt_rms */
/* input		::	val_a,val_b,val_c		(bcd) */
/* output		::	calculation total value *10	 (0.1w) (bcd) */
/* call			::	met_utils_bcd_8bits_add */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_PQSt_rms( uint32_t val_a, uint32_t val_b, uint32_t val_c )
{
	uint32_t a, b, c, t;
	a = val_a;
	b = val_b;
	c = val_c;
	t = 0;
	met_utils_bcd_8bits_add((uint8_t *)(&a), (uint8_t *)(&t));
	met_utils_bcd_8bits_add((uint8_t *)(&b), (uint8_t *)(&t));
	met_utils_bcd_8bits_add((uint8_t *)(&c), (uint8_t *)(&t));

	return (t);
}

/* =================================================================== */
/* description	::	check active or reactive power direction */
/* function		::	check_PQ_direc */
/* input		::	p,q */
/* output		::	1:reverse, 0:forward */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t check_PQ_direc( int64_t val )
{
	if (val >= (0x8000000000000000)) {
		return 1;
	} else {
		return 0;
	}
}

/* =================================================================== */
/* description	::	check active or reactive total power direction */
/* function		::	check_PQt_direc */
/* input		::	pa pb pc */
/* output		::	1:reverse, 0:forward */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t        check_PQt_direc( int64_t val_a, int64_t val_b, int64_t val_c )
{
	if ((val_a + val_b + val_c) >= (0x8000000000000000)) {
		return 1;
	} else {
		return 0;
	}
}

/* =================================================================== */
/* description	::	measure rms value */
/* function		::	measure_rms */
/* input		::	rms_id */
/* output		::	U = xxxx.xxxx (V)               I = xxxx.xxxx (A) */
/*                              P = xxxxxxx.x (W)		Q = xxxxxxx.x (Var) */
/*                              Fr= xx.xx (Hz) */
/* call			::	calculate_VI_rms,calculate_PQ_rms,check_PQ_direc */
/* effect		::	VAFE */
/* =================================================================== */
uint32_t measure_rms( RMS_TYPE rms_id )
{
	/* uint32_t	i; */
	switch (rms_id) {
	case Ua:
	{
		VAFE.RMS[Ua] = calculate_VI_rms( VMetrology.DSP_ACC.V_A, VMetrology.DSP_CTRL.K_VA );
	}
	break;

	case Ub:
	{
		VAFE.RMS[Ub] = calculate_VI_rms( VMetrology.DSP_ACC.V_B, VMetrology.DSP_CTRL.K_VB );
	}
	break;

	case Uc:
	{
		VAFE.RMS[Uc] = calculate_VI_rms( VMetrology.DSP_ACC.V_C, VMetrology.DSP_CTRL.K_VC );
	}
	break;

	case Ia:
	{
		VAFE.RMS[Ia] = calculate_VI_rms( VMetrology.DSP_ACC.I_A, VMetrology.DSP_CTRL.K_IA );
	}
	break;

	case Ib:
	{
		VAFE.RMS[Ib] = calculate_VI_rms( VMetrology.DSP_ACC.I_B, VMetrology.DSP_CTRL.K_IB );
	}
	break;

	case Ic:
	{
		VAFE.RMS[Ic] = calculate_VI_rms( VMetrology.DSP_ACC.I_C, VMetrology.DSP_CTRL.K_IC );
	}
	break;

	case Pa:
	{
		/* VAFE.RMS[Pa] = calculate_PQ_rms_bcd( VMetrology.DSP_ACC.P_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA ); */
		VAFE.RMS[Pa] = calculate_PQ_rms( VMetrology.DSP_ACC.P_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA );
		VAFE.ST.BIT.pa_dir = check_PQ_direc( VMetrology.DSP_ACC.P_A );
	}
	break;

	case Pb:
	{
		/* VAFE.RMS[Pb] = calculate_PQ_rms_bcd( VMetrology.DSP_ACC.P_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB ); */
		VAFE.RMS[Pb] = calculate_PQ_rms( VMetrology.DSP_ACC.P_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB );
		VAFE.ST.BIT.pb_dir = check_PQ_direc( VMetrology.DSP_ACC.P_B );
	}
	break;

	case Pc:
	{
		/* VAFE.RMS[Pc] = calculate_PQ_rms_bcd( VMetrology.DSP_ACC.P_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC ); */
		VAFE.RMS[Pc] = calculate_PQ_rms( VMetrology.DSP_ACC.P_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC );
		VAFE.ST.BIT.pc_dir = check_PQ_direc( VMetrology.DSP_ACC.P_C );
	}
	break;

	case Qa:
	{
		/* VAFE.RMS[Qa] = calculate_PQ_rms_bcd( VMetrology.DSP_ACC.Q_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA ); */
		VAFE.RMS[Qa] = calculate_PQ_rms( VMetrology.DSP_ACC.Q_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA );
		VAFE.ST.BIT.qa_dir = check_PQ_direc( VMetrology.DSP_ACC.Q_A );
	}
	break;

	case Qb:
	{
		/* VAFE.RMS[Qb] = calculate_PQ_rms_bcd( VMetrology.DSP_ACC.Q_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB ); */
		VAFE.RMS[Qb] = calculate_PQ_rms( VMetrology.DSP_ACC.Q_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB );
		VAFE.ST.BIT.qb_dir = check_PQ_direc( VMetrology.DSP_ACC.Q_B );
	}
	break;

	case Qc:
	{
		/* VAFE.RMS[Qc] = calculate_PQ_rms_bcd( VMetrology.DSP_ACC.Q_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC ); */
		VAFE.RMS[Qc] = calculate_PQ_rms( VMetrology.DSP_ACC.Q_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC );
		VAFE.ST.BIT.qc_dir = check_PQ_direc( VMetrology.DSP_ACC.Q_C );
	}
	break;

	case Sa:
	{
		/* VAFE.RMS[Sa] = calculate_S_rms_bcd( VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.Q_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA ); */
		VAFE.RMS[Sa] = calculate_S_rms( VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.Q_A, VMetrology.DSP_CTRL.K_IA, VMetrology.DSP_CTRL.K_VA );
	}
	break;

	case Sb:
	{
		/* VAFE.RMS[Sb] = calculate_S_rms_bcd( VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.Q_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB ); */
		VAFE.RMS[Sb] = calculate_S_rms( VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.Q_B, VMetrology.DSP_CTRL.K_IB, VMetrology.DSP_CTRL.K_VB );
	}
	break;

	case Sc:
	{
		/* VAFE.RMS[Sc] = calculate_S_rms_bcd( VMetrology.DSP_ACC.P_C, VMetrology.DSP_ACC.Q_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC ); */
		VAFE.RMS[Sc] = calculate_S_rms( VMetrology.DSP_ACC.P_C, VMetrology.DSP_ACC.Q_C, VMetrology.DSP_CTRL.K_IC, VMetrology.DSP_CTRL.K_VC );
	}
	break;

	case Pt:
	{
		VAFE.RMS[Pt] = calculate_PQSt_rms( VAFE.RMS[Pa], VAFE.RMS[Pb], VAFE.RMS[Pc] );
		VAFE.ST.BIT.pt_dir = check_PQt_direc( VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.P_C );
	}
	break;

	case Qt:
	{
		VAFE.RMS[Qt] = calculate_PQSt_rms( VAFE.RMS[Qa], VAFE.RMS[Qb], VAFE.RMS[Qc] );
		VAFE.ST.BIT.qt_dir = check_PQt_direc( VMetrology.DSP_ACC.Q_A, VMetrology.DSP_ACC.Q_B, VMetrology.DSP_ACC.Q_C );
	}
	break;

	case St:
	{
		VAFE.RMS[St] = calculate_PQSt_rms( VAFE.RMS[Sa], VAFE.RMS[Sb], VAFE.RMS[Sc] );
	}
	break;

	case Freq:
	{
		VAFE.RMS[Freq] = (uint32_t)(met_utils_hex_4byte_to_bcd(((VMetrology.DSP_ST.FREQ * 100) >> FREQ_Q)));       /* xx.xx (Hz) */
	}
	break;

	case AngleA:
	{
		VAFE.RMS[AngleA] = (uint32_t)(met_utils_hex_4byte_to_bcd( calculate_Angle_rms( VMetrology.DSP_ACC.P_A, VMetrology.DSP_ACC.Q_A )));
	}
	break;

	case AngleB:
	{
		VAFE.RMS[AngleB] = (uint32_t)(met_utils_hex_4byte_to_bcd( calculate_Angle_rms( VMetrology.DSP_ACC.P_B, VMetrology.DSP_ACC.Q_B )));
	}
	break;

	case AngleC:
	{
		VAFE.RMS[AngleC] = (uint32_t)(met_utils_hex_4byte_to_bcd( calculate_Angle_rms( VMetrology.DSP_ACC.P_C, VMetrology.DSP_ACC.Q_C )));
	}
	break;
	}
	return (1);
}

/* =================================================================== */
/* description	::	calculate active or reactive energy */
/* function		::	calculate_pq_energy */
/* input		::	mode: 1=alg,0=abs */
/*				::	id:	0=active,1=reactive */
/* output		::	energy (wh) */
/* call			::	none */
/* effect		::	none */
/* =================================================================== */
uint32_t calculate_pq_energy( M_ENERGY_TYPE id, M_ENERGY_TYPE mode )
{
	double m, k;
	m = 0;
	k = 0;
	if (id == PEnergy) {
		if (mode == AbsAdd) {
			if (VMetrology.DSP_ACC.P_A >= (RMS_PQ_SYMB)) {
				m = (double)((~VMetrology.DSP_ACC.P_A) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.P_A);
			}

			m = (m * VMetrology.DSP_CTRL.K_IA * VMetrology.DSP_CTRL.K_VA) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);                    /* k =k/2^40 */
			/* k += m / 4000;                       / * k =k/4000 * / */
			k += (m / VMetrology.DSP_ST.N);
			if (VMetrology.DSP_ACC.P_B >= (RMS_PQ_SYMB)) {
				m = (double)((~VMetrology.DSP_ACC.P_B) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.P_B);
			}

			m = (m * VMetrology.DSP_CTRL.K_IB * VMetrology.DSP_CTRL.K_VB) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);                    /* k =k/2^40 */
			/* k += m / 4000;                       / * k =k/4000 * / */
			k += (m / VMetrology.DSP_ST.N);
			if (VMetrology.DSP_ACC.P_C >= (RMS_PQ_SYMB)) {
				m += (double)((~VMetrology.DSP_ACC.P_C) + 1);
			} else {
				m += (double)(VMetrology.DSP_ACC.P_C);
			}

			m = (m * VMetrology.DSP_CTRL.K_IC * VMetrology.DSP_CTRL.K_VC) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);                    /* k =k/2^40 */
			/* k += m / 4000;                       / * k =k/4000 * / */
			k += (m / VMetrology.DSP_ST.N);
		} /* abs */
		else {
		} /* algebra */
	} /* active energy */
	else {
		if (mode == 0) {
			if (VMetrology.DSP_ACC.Q_A >= (RMS_PQ_SYMB)) {
				m = (double)((~VMetrology.DSP_ACC.Q_A) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.Q_A);
			}

			m = (m * VMetrology.DSP_CTRL.K_IA * VMetrology.DSP_CTRL.K_VA) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);                    /* k =k/2^40 */
			/* k += m / 4000;                       / * k =k/4000 * / */
			k += (m / VMetrology.DSP_ST.N);
			if (VMetrology.DSP_ACC.Q_B >= (RMS_PQ_SYMB)) {
				m = (double)((~VMetrology.DSP_ACC.Q_B) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.Q_B);
			}

			m = (m * VMetrology.DSP_CTRL.K_IB * VMetrology.DSP_CTRL.K_VB) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);                    /* k =k/2^40 */
			/* k += m / 4000;                       / * k =k/4000 * / */
			k += (m / VMetrology.DSP_ST.N);
			if (VMetrology.DSP_ACC.Q_C >= (RMS_PQ_SYMB)) {
				m = (double)((~VMetrology.DSP_ACC.Q_C) + 1);
			} else {
				m = (double)(VMetrology.DSP_ACC.Q_C);
			}

			m = (m * VMetrology.DSP_CTRL.K_IC * VMetrology.DSP_CTRL.K_VC) / (RMS_DIV_G * RMS_DIV_G); /* m =m*k_v*k_i */
			m = (m / RMS_DIV_Q);                    /* k =k/2^40 */
			/* k += m / 4000;                       / * k =k/4000 * / */
			k += (m / VMetrology.DSP_ST.N);
		} /* abs */
		else {
		} /* algebra */
	} /* reactive energy */

	/* k =(k/RMS_DIV_Q);                                    / * k =k/2^40 * / */
	/* k =k/4000;                                           / * k =k/4000 * / */
	k = k / 3600;                                           /* xxxxxx (Wh/Varh) */
	k = k * 10000;                                          /* *10000 (kWh/kVarh) */

	return ((uint32_t)(k));                                 /* xxxx (kWh/kVarh) */
}

/* =================================================================== */
/* description	::	metrology data refresh process */
/* function		::	metrology_data_refresh_proc */
/* input		::	none */
/* output		::	none */
/* call			::	calculate_VI_rms,calculate_PQ_rms,check_PQ_direc */
/* effect		::	VAFE, */
/* =================================================================== */
void metrology_data_refresh_proc( void )
{
	__disable_irq();
	VAFE.updataflag = 0;
	__enable_irq();

	if (VCal_Meter.dsp_update_num < 10 && VCal_Meter.dsp_update_num > 4) {
		VCal_Meter.dsp_acc_ia += VMetrology.DSP_ACC.I_A;
		VCal_Meter.dsp_acc_ib += VMetrology.DSP_ACC.I_B;
		VCal_Meter.dsp_acc_ic += VMetrology.DSP_ACC.I_C;

		VCal_Meter.dsp_acc_ua += VMetrology.DSP_ACC.V_A;
		VCal_Meter.dsp_acc_ub += VMetrology.DSP_ACC.V_B;
		VCal_Meter.dsp_acc_uc += VMetrology.DSP_ACC.V_C;

		VCal_Meter.dsp_acc_pa += VMetrology.DSP_ACC.P_A;
		VCal_Meter.dsp_acc_pb += VMetrology.DSP_ACC.P_B;
		VCal_Meter.dsp_acc_pc += VMetrology.DSP_ACC.P_C;

		VCal_Meter.dsp_acc_qa += VMetrology.DSP_ACC.Q_A;
		VCal_Meter.dsp_acc_qb += VMetrology.DSP_ACC.Q_B;
		VCal_Meter.dsp_acc_qc += VMetrology.DSP_ACC.Q_C;
	}

	if (VCal_Meter.dsp_update_num != 0) {
		if ((--VCal_Meter.dsp_update_num) == 5) {
			VCal_Meter.dsp_update_num = 3;
			/* -------triggle calibration msg--------- */
			/* PutTaskIntoQue( CalMeter ); */
			/* --------------------------------------- */
			/* Calbrate_Meter( &VCal_Meter); */
		}
	}

	measure_rms( Ua );
	measure_rms( Ub );
	measure_rms( Uc );

	measure_rms( Ia );
	measure_rms( Ib );
	measure_rms( Ic );

	measure_rms( Pa );
	measure_rms( Pb );
	measure_rms( Pc );

	measure_rms( Qa );
	measure_rms( Qb );
	measure_rms( Qc );

	measure_rms( Sa );
	measure_rms( Sb );
	measure_rms( Sc );

	measure_rms( Freq );

	measure_rms( Pt );
	measure_rms( Qt );
	measure_rms( St );

	measure_rms( AngleA );
	measure_rms( AngleB );
	measure_rms( AngleC );

	VAFE.energy += calculate_pq_energy( PEnergy, AbsAdd );  /* (wh) */
}

/* =================================================================== */
/* description	::	clear afe rms */
/* function		::	clear_afe_rms */
/* input		::	none */
/* output		::	none */
/* call			::	none */
/* effect		::	VAFE, */
/* =================================================================== */
void clear_afe_rms( void )
{
	uint32_t i;
	uint32_t *ptr;
	ptr = &VAFE.RMS[Ia];
	for (i = 0; i < 14; i++) {
		*(ptr + i) = 0;
	}
}

uint32_t metrology_get_rms_value(RMS_TYPE rms_id)
{
	return VAFE.RMS[rms_id];
}

void metrology_init(void)
{
	/* Load default metrology values */
	DSP_Load_Default();

	/* Init DSP core */
	dsp_init();
}

void metrology_get_measures(uint32_t *pul_met_data)
{
	memcpy(pul_met_data, VAFE.RMS, sizeof(VAFE));
}

void metrology_update_met_data_set_callback(void (*pf_event_cb)(void))
{
	ipc_integration_event_set_callback(pf_event_cb);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
