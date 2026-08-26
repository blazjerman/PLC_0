/**
 * \file
 *
 * \brief Energy Module file.
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

#include <string.h>
#include "compiler.h"
#include "tou.h"
#include "extmem.h"
#include "metrology.h"
#include "energy.h"
#include "utils.h"

energy_t VEnergy;
energy_ctrl_t VEnergyCtrl;

/**
 * \brief Energy initialize.
 */
void EnergyInit(uint32_t energy_mem_threshold)
{
	energy_t temp;

	/* Update Energy data to External memory */
	ExtMemRead(MEM_REG_ENERGY_ID, &temp);

	if (temp.tou1_acc == 0xFFFFFFFF) {
		/* Init External Memory region */
		memset(&VEnergy, 0, sizeof(energy_t));
		/* Update Energy data to External memory */
		ExtMemWrite(MEM_REG_ENERGY_ID, &VEnergy);
	} else {
		/* Restore energy data from memory data */
		VEnergy = temp;
	}

	/* Init Energy control parameters */
	VEnergyCtrl.mem_update = MEM_UPDATE_NO;
        VEnergyCtrl.energy_threshold=energy_mem_threshold;
}

/**
 * \brief Energy Process.
 */
void EnergyProcess(void)
{
	uint64_t *ptr_acc;
	energy_t temp;

	/* Select TOU rate_id */
	if (VTou.time_slot[VTou.time_slot_idx].rate_id == 0) {
		ptr_acc = &VEnergy.tou1_acc;
	} else if (VTou.time_slot[VTou.time_slot_idx].rate_id == 1) {
		ptr_acc = &VEnergy.tou2_acc;
	} else if (VTou.time_slot[VTou.time_slot_idx].rate_id == 2) {
		ptr_acc = &VEnergy.tou3_acc;
	} else {
		ptr_acc = &VEnergy.tou4_acc;
	}

	*ptr_acc += VAFE.energy;
	VAFE.energy = 0;

	/* Check if Energy values should be updated in external memory */
	if (VEnergyCtrl.mem_update == MEM_UPDATE_PERIODICAL) {
		VEnergyCtrl.mem_update = MEM_UPDATE_NO;
		/* Read memory */
		ExtMemRead(MEM_REG_ENERGY_ID, &temp);
		/* Update memory if any accumulator has increased more than TOU_MEM_THRESHOLD */
		if ( (VEnergy.tou1_acc + VEnergy.tou2_acc + VEnergy.tou3_acc + VEnergy.tou4_acc - temp.tou1_acc - temp.tou2_acc - temp.tou3_acc - temp.tou4_acc)>= VEnergyCtrl.energy_threshold) {
			/* Update Energy data to External memory */
			ExtMemWrite(MEM_REG_ENERGY_ID, &VEnergy);
		}
	} else if (VEnergyCtrl.mem_update == MEM_UPDATE_URGENT) {
		VEnergyCtrl.mem_update = MEM_UPDATE_NO;
		/* Read memory */
		ExtMemRead(MEM_REG_ENERGY_ID, &temp);
		/* Update memory if any accumulator has changed */
                if ( (VEnergy.tou1_acc!=temp.tou1_acc) || (VEnergy.tou2_acc!=temp.tou2_acc) ||(VEnergy.tou3_acc!=temp.tou3_acc) ||(VEnergy.tou4_acc!=temp.tou4_acc) ) {
                        ExtMemWrite(MEM_REG_ENERGY_ID, &VEnergy);
                }
        }

	LOG_APP_DEMO_DEBUG(("Energy Process: Update VEnergy.tou%d_acc[0x%08x]\r\n", VTou.time_slot_idx+1, (unsigned int)*ptr_acc));
}

/**
 * \brief Write Energy data to external memory.
 */
void EnergyWriteExternalMem(void)
{
	/* Update Energy data to External memory */
	ExtMemWrite(MEM_REG_ENERGY_ID, &VEnergy);
}

/**
 * \brief Clear Energy data.
 */
void EnergyClear(void)
{
	/* Clear Energy data */
	memset(&VEnergy, 0, sizeof(VEnergy));
	/* Clear Energy data from External memory */
	ExtMemWrite(MEM_REG_ENERGY_ID, &VEnergy);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
