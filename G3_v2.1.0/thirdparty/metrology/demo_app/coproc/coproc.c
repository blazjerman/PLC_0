/**
 * \file
 *
 * \brief Coprocessor Module file.
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

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "string.h"
#include "asf.h"
#include "metrology.h"
#include "coproc.h"
#include "utils.h"

#if defined (__ICCARM__)
extern char core1_metlib;
#elif defined (__CC_ARM)
extern uint8_t core1_metlib_start;
extern uint8_t core1_metlib_end;
#endif

DSP_CTRL_TYPE *ptr_mem_reg_in;
DSP_ST_TYPE *ptr_mem_reg_out;
DSP_ACC_TYPE *ptr_mem_acc_out;
DSP_HAR_TYPE *ptr_mem_har_out;

/* Region descriptor in main list */
COMPILER_ALIGNED(64)
	struct icm_region_descriptor_main_list reg_descriptor;

/* Hash area */
COMPILER_ALIGNED(128)
	uint32_t iram1_sha[0x20];

COMPILER_ALIGNED(128)
	uint32_t met_lib_bin_sha[0x20];

static volatile uint32_t gs_icm_hash_invoked = 0;

static void IPC1_INIT_INT_Handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
	VMetrology.DSP_INIT_FLAG = 0x68;
}

#if (defined(USE_MIKROBUS_PIN_RST_FOR_RZC_TOGGLE) || defined(USE_MIKROBUS_PIN_RST_FOR_RZC_SQUARE_OUT))
static void IPC1_RZC_INT_Handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);

#ifdef USE_MIKROBUS_PIN_RST_FOR_RZC_TOGGLE
	/* toggle IO pin MIKROBUS_PIN_RST performed here in Core-0 for debug pulses */
	ioport_toggle_pin_level(MIKROBUS_PIN_RST);
#endif /* #ifdef USE_PA28_FOR_RZC_TOGGLE */

#ifdef USE_MIKROBUS_PIN_RST_FOR_RZC_SQUARE_OUT
	/* set IO pin MIKROBUS_PIN_RST for +RZC and clear for -RZC. performed here in Core-0 for debug pulses */
	if (ptr_mem_reg_in->FEATURE_CTRL0.BIT.RZC_DIR == 1) {
		/* clear bit if -RZC is selected */
		ioport_set_pin_level(MIKROBUS_PIN_RST, 0);
		/* now select +RZC */
		ptr_mem_reg_in->FEATURE_CTRL0.BIT.RZC_DIR = 0;
	} else {
		/* set bit if +RZC is selected */
		ioport_set_pin_level(MIKROBUS_PIN_RST, 1);
		/* now select -RZC */
		ptr_mem_reg_in->FEATURE_CTRL0.BIT.RZC_DIR = 1;
	}
#endif /* #ifdef USE_PA28_FOR_RZC_SQUARE_OUT */
}

#endif /* #if (defined(USE_MIKROBUS_PIN_RST_FOR_RZC_TOGGLE) || defined(USE_MIKROBUS_PIN_RST_FOR_RZC_SQUARE_OUT)) */

static void IPC1_HALF_CYCLE_INT_Handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
}

static void IPC1_FULL_CYCLE_INT_Handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
}

static void IPC1_INTEGRATION_INT_Handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);

	if (VAFE.updataflag) {
#if defined(APP_DEMO_DEBUG_CONSOLE)
		LOG_APP_DEMO_DEBUG(("IPC1_INT ERROR : data underrun\r\n"));
#elif defined(APP_DEMO_DEBUG_WDG0_RESTART)
		LOG_APP_DEMO_WDG0_DEBUG(("IPC1_INT ERROR : data underrun\r\n"));
#endif
	}

	VAFE.updataflag = 1;

	/* Update DSP Status */
	UtilsMemCpy((void *)&VMetrology.DSP_STATUS, (void *)ptr_mem_reg_out, sizeof(VMetrology.DSP_STATUS));

	if (VMetrology.DSP_STATUS.STATUS.BIT.ST == STATUS_DSP_RUNNING) {
		/* update DSP ACC */
		UtilsMemCpy((void *)&VMetrology.DSP_ACC, (void *)ptr_mem_acc_out, sizeof(VMetrology.DSP_ACC));
		/* update DSP HAR */
		UtilsMemCpy((void *)&VMetrology.DSP_HAR, (void *)ptr_mem_har_out, sizeof(VMetrology.DSP_HAR));
	}

	/* Update FEATURE_CTRL1 register (metrology library clears the V_MAX_RESET e I_MAX_RESET flags */
	VMetrology.DSP_CTRL.FEATURE_CTRL1 = ptr_mem_reg_in->FEATURE_CTRL1;
}

static void _reg_dig_mismatch_handler(uint8_t reg_num)
{
	if(reg_num == ICM_REGION_NUM_0) {
		/* MCU reset. Additional actions could be done: reload metrology library without restarting MCU,
		count number of mismatches and store it in NVM, ... */
		rstc_start_software_reset( RSTC );
	}
}

static void  _region_hash_completed_handler(uint8_t reg_num)
{
	if(reg_num == ICM_REGION_NUM_0) {
		gs_icm_hash_invoked=1;
	}
}

static void _region_bus_error_handler(uint8_t reg_num)
{
	if(reg_num == ICM_REGION_NUM_0) {
		/* MCU reset. Additional actions could be done: reload metrology library without restarting MCU,
		count number of mismatches and store it in NVM, ... */
		rstc_start_software_reset( RSTC );
	}
}

static void _ICM_monitoring_init (uint32_t ul_address, uint16_t us_size)
{
	/* ICM configuration */
	struct icm_config icm_cfg;

	/* ---------------------------------------------------------------- */
	/* Step 1: Use the ICM to compute the metrology library binary Hash */
	/* ---------------------------------------------------------------- */

	/* ICM initialization */
	icm_cfg.is_write_back= false;
	icm_cfg.is_dis_end_mon = false;
	icm_cfg.is_sec_list_branch = false;
	icm_cfg.bbc = 0;
	icm_cfg.is_auto_mode = false;
	icm_cfg.is_dual_buf = false;
	icm_cfg.is_user_hash = false;
	icm_cfg.ualgo = ICM_SHA_1;
	icm_cfg.hash_area_val = 1;
	icm_cfg.des_area_val = 1;
	icm_init(ICM, &icm_cfg);

	/* Set region0 descriptor */
	/* Test: use an incorrect start address to force a mismatch of the IRAM1 and Flash hashes*/
	/* reg_descriptor.start_addr = ul_src_bin + 8; */
#if defined (__ICCARM__)
	reg_descriptor.start_addr = (int)&core1_metlib;
#elif defined (__CC_ARM)
	reg_descriptor.start_addr = (int)(&core1_metlib_start - 1);
#endif
	reg_descriptor.cfg.is_compare_mode = false;
	reg_descriptor.cfg.is_wrap = false;
	reg_descriptor.cfg.is_end_mon = true;
	reg_descriptor.cfg.reg_hash_int_en = false;
	reg_descriptor.cfg.dig_mis_int_en = false;
	reg_descriptor.cfg.bus_err_int_en = false;
	reg_descriptor.cfg.wrap_con_int_en = false;
	reg_descriptor.cfg.ebit_con_int_en = false;
	reg_descriptor.cfg.status_upt_con_int_en = false;
	reg_descriptor.cfg.is_pro_dly = false;
	reg_descriptor.cfg.mem_reg_val = 1;
	reg_descriptor.cfg.algo = ICM_SHA_1;
	reg_descriptor.tran_size = us_size >> 6;
	if (us_size & 0x3F) {
		reg_descriptor.tran_size++;
		/* The binary size is not multiple of 64 bytes, so (64-us_size >> 6) extra bytes should be written in IRAM1 to ensure the hashes match */
		uint32_t i, j;
		j = 64- (us_size%64);
#if defined (__ICCARM__)
		#pragma section = "P_core1_metlib"
		uint32_t *pul_src = (uint32_t *)__section_end("P_core1_metlib");
#elif defined (__CC_ARM)
		uint32_t *pul_src = (uint32_t *)&core1_metlib_end;
#endif
		uint32_t *pul_dst = (uint32_t *)(ul_address + us_size);
		__disable_irq();
		for (i = 0; i < j; i += 4, pul_src++, pul_dst++) {
			*pul_dst = *pul_src;
		}
		__enable_irq();
	}
	reg_descriptor.tran_size--;
	reg_descriptor.next_addr = NULL;

	/* Set region descriptor start address */
	icm_set_reg_des_addr(ICM, (uint32_t)&reg_descriptor);

	/* Set hash area start address */
	icm_set_hash_area_addr(ICM, (uint32_t)met_lib_bin_sha);

	/* Set callback function for region hash complete interrupt handler */
	icm_set_callback(ICM, _region_hash_completed_handler, ICM_REGION_NUM_0, ICM_INTERRUPT_RHC, 1);

	/* Enable ICM */
	icm_enable(ICM);
	while(!gs_icm_hash_invoked);	/* wait till hash complete */
	icm_disable(ICM);
	gs_icm_hash_invoked=0;

	/* -------------------------------------------- */
	/* Step 2: Activate IRAM1 continuous monitoring */
	/* -------------------------------------------- */

	/* ICM initialization */
	icm_cfg.bbc = 15;	/* Minimize bandwidth usage */
	icm_cfg.is_auto_mode = true;
	icm_init(ICM, &icm_cfg);

	/* Set region0 descriptor */
	reg_descriptor.start_addr = ul_address;
	reg_descriptor.cfg.is_compare_mode = true;
	reg_descriptor.cfg.is_wrap = true;
	reg_descriptor.cfg.is_end_mon = false;
	reg_descriptor.cfg.is_pro_dly = true;       /* Minimize bandwidth usage */

	/* Set region descriptor start address */
	icm_set_reg_des_addr(ICM, (uint32_t)&reg_descriptor);

	/* Set hash area start address */
	icm_set_hash_area_addr(ICM, (uint32_t)iram1_sha);

	/* Set callback function for digest mismatch interrupt handler */
	icm_set_callback(ICM, _reg_dig_mismatch_handler, ICM_REGION_NUM_0,ICM_INTERRUPT_RDM, 1);
	icm_set_callback(ICM, _region_bus_error_handler, ICM_REGION_NUM_0,ICM_INTERRUPT_RBE, 1);
	icm_set_callback(ICM, _region_hash_completed_handler, ICM_REGION_NUM_0,ICM_INTERRUPT_RHC, 1);

	/* Enable ICM */
	icm_enable(ICM);

	/* ------------------------------------ */
	/* Step 3: Ensure that the hashes match */
	/* ------------------------------------ */
	/* Wait until the IRAM1 hash is calculated */
	while(!gs_icm_hash_invoked);
	/* Compare the two hashes (number of bytes depending on the algorithm used) */
	int i;
	for (i=0;i<8;i++) {
		if (iram1_sha[i] != met_lib_bin_sha[i]) {
			/* MCU reset. Additional actions could be done: reload metrology library without restarting MCU,
			count number of mismatches and store it in NVM, ... */
			rstc_start_software_reset( RSTC );
		}
	}

	/* Test: modify memory region value to trigger the mismatch interrupt*/
	/* uint32_t *write_ptr = (uint32_t *)IRAM1_ADDR;
	*write_ptr=0x12345678; */
}

/**
*  \brief Copy Core 1 image into SRAM1.
*/
static void _copy_core1_image_into_sram1(bool monitor_en)
{
#if defined (__ICCARM__)
	#pragma section = "P_core1_metlib"
	uint32_t *pul_src = (uint32_t *)&core1_metlib;
	uint32_t *puc_bin_end = (uint32_t *)__section_end("P_core1_metlib");
#elif defined (__CC_ARM)
	uint32_t *pul_src = (uint32_t *)(&core1_metlib_start - 1);
	uint32_t *puc_bin_end = (uint32_t *)&core1_metlib_end;
#endif
	uint32_t *pul_dst = (uint32_t *)IRAM1_ADDR;
	uint32_t i, ul_bin_size;

	ul_bin_size = ((uint32_t)puc_bin_end - (uint32_t)pul_src);

	/* Disable ICM */
	icm_disable(ICM);

	__disable_irq();

	for (i = 0; i < ul_bin_size; i += 4, pul_src++, pul_dst++) {
		*pul_dst = *pul_src;
	}

	__enable_irq();

	if (monitor_en) {
		_ICM_monitoring_init(IRAM1_ADDR, (uint16_t )ul_bin_size);
	}
}

/**
*  \brief Configure the IPC.
*/
static void _configure_ipc(void)
{
	/* Enable IPC1 clock */
	ipc_enable(IPC1);

	/* Clear IPC commands */
	ipc_clear_command(IPC1, IPC_INIT_IRQ);
	ipc_clear_command(IPC1, IPC_HALF_CYCLE_IRQ);
	ipc_clear_command(IPC1, IPC_FULL_CYCLE_IRQ);
	ipc_clear_command(IPC1, IPC_INTEGRATION_IRQ);

	/* Set IPC interrupt source */
	ipc_set_handler(IPC1, IPC_INIT_IRQ, IPC1_INIT_INT_Handler);
	ipc_set_handler(IPC1, IPC_HALF_CYCLE_IRQ, IPC1_HALF_CYCLE_INT_Handler);
	ipc_set_handler(IPC1, IPC_FULL_CYCLE_IRQ, IPC1_FULL_CYCLE_INT_Handler);
	ipc_set_handler(IPC1, IPC_INTEGRATION_IRQ, IPC1_INTEGRATION_INT_Handler);

#if (defined(USE_MIKROBUS_PIN_RST_FOR_RZC_TOGGLE) || defined(USE_MIKROBUS_PIN_RST_FOR_RZC_SQUARE_OUT))
	ipc_set_handler(IPC1, IPC_RZC_IRQ, IPC1_RZC_INT_Handler);
	ipc_enable_interrupt(IPC1, IPC_INIT_IRQ | IPC_HALF_CYCLE_IRQ | IPC_FULL_CYCLE_IRQ | IPC_INTEGRATION_IRQ | IPC_RZC_IRQ);
#else
	ipc_enable_interrupt(IPC1, IPC_INIT_IRQ | IPC_HALF_CYCLE_IRQ | IPC_FULL_CYCLE_IRQ | IPC_INTEGRATION_IRQ);
#endif
	/* Enable IPC1 interrupt */
	NVIC_DisableIRQ(IPC1_IRQn);
	NVIC_ClearPendingIRQ(IPC1_IRQn);
	NVIC_EnableIRQ(IPC1_IRQn);
}

/**
*  \brief Initialization of the interface with Core 1.
*/
void CoprocInit(bool monitor_en, reset_type_t reset_type)
{
	metrology_t *pVMetrology;

	if (reset_type == RESET_TYPE_HARD) {
                /* Assert reset of core-1 processor & peripherals */
                rstc_assert_reset_of_coprocessor(RSTC, RSTC_MR_CPROCEN | RSTC_MR_CPEREN); 
		/* Disable coprocessor clock */
                pmc_disable_cpck(); 
		/* Disable coprocessor Bus Master Clocks (peripheral clocks will be also disabled) */
                pmc_disable_cpbmck(); 
		/* Deassert core-1 peripheral reset */
                rstc_deassert_reset_of_coprocessor(RSTC, RSTC_MR_CPEREN);   
		/* Enable coprocessor Bus Master Clocks */
                pmc_enable_cpbmck(); 
	}

	/* Init Pointers to shared memory data */
	pVMetrology = (metrology_t *)MEM_REG_IN_ADDRESS;
	ptr_mem_reg_in  = (DSP_CTRL_TYPE *)&pVMetrology->DSP_CTRL;
	ptr_mem_reg_out = (DSP_ST_TYPE *)&pVMetrology->DSP_STATUS;
	ptr_mem_acc_out = (DSP_ACC_TYPE *)&pVMetrology->DSP_ACC;
	ptr_mem_har_out = (DSP_HAR_TYPE *)&pVMetrology->DSP_HAR;

	_configure_ipc(); 

	if (reset_type == RESET_TYPE_HARD) {
		/* Clear metrology control register structure */
		memset((uint8_t *)ptr_mem_reg_in, 0, sizeof(DSP_CTRL_TYPE));
		memset((uint8_t *)ptr_mem_reg_out, 0, sizeof(DSP_ST_TYPE));
		memset((uint8_t *)ptr_mem_acc_out, 0, sizeof(DSP_ACC_TYPE));
		memset((uint8_t *)ptr_mem_har_out, 0, sizeof(DSP_HAR_TYPE));
		VMetrology.DSP_INIT_FLAG = 0;

		_copy_core1_image_into_sram1(monitor_en);
        	/* Release Coprocessor reset */
                rstc_deassert_reset_of_coprocessor(RSTC, RSTC_MR_CPROCEN); 
		/* Enable coprocessor clock */
                pmc_enable_cpck(); 
	}

	if (reset_type == RESET_TYPE_SOFT) {
		UtilsMemCpy((void *)&VMetrology.DSP_CTRL, (void *)ptr_mem_reg_in, sizeof(VMetrology.DSP_CTRL));
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
