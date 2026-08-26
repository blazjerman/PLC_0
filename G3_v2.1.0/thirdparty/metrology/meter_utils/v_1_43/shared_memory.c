/* =================================================================== */
#include "asf.h"
#include "shared_memory.h"
#include "metrology.h"
/* =================================================================== */

__IO uint32_t *Metrology_Reg_In;
__IO uint32_t *Metrology_Reg_Out;
__IO uint32_t *Metrology_Acc_Out;
__IO uint32_t *Metrology_Har_Out;

static void (*spf_ipc_integration_event_cb)(void);

/* =================================================================== */
/* description	        ::	core interrupt process */
/* function		::	IPC_Handler */
/* input		::	none */
/* output		::	none */
/* call			::	ipc_get_status,ipc_clear_interrupt */
/* effect		::	VMetrology */
/* =================================================================== */

void ipc_init_irq_handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
}

void ipc_status_handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
}

void ipc_half_cycle_handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
}

void ipc_full_cycle_irq_handler(Ipc *p, enum ipc_interrupt_source mask)
{
	ipc_clear_command(p, mask);
}

void ipc_integration_irq_handler(Ipc *p, enum ipc_interrupt_source mask)
{
	uint32_t i;

	ipc_clear_command(p, mask);
	VAFE.updataflag = 0x68;

	for (i = 0; i < DSP_ST_SIZE; i++) {
		*((uint32_t *)(&VMetrology.DSP_ST.VERSION.WORD) + i) = *((uint32_t *)(mem_reg_out) + i);
	}

	if (((volatile DSP_ST_TYPE *)(mem_reg_out))->STATUS.BIT.ST == DSP_ST_DSP_Running) {
		for (i = 0; i < (DSP_ACC_SIZE * 2); i = i + 2) {
			*((uint32_t *)(&VMetrology.DSP_ACC.I_A) + i) = *((uint32_t *)(mem_acc_out) + i);
			*((uint32_t *)(&VMetrology.DSP_ACC.I_A) + i + 1) = *((uint32_t *)(mem_acc_out) + i + 1);
		}

		for (i = 0; i < (DSP_HAR_SIZE); i++) {
			*((uint32_t *)(&VMetrology.DSP_HAR.I_A_R) + i) = *((uint32_t *)(mem_har_out) + i);
		}
	}

	/* Report new data capture */
	if (spf_ipc_integration_event_cb) {
		spf_ipc_integration_event_cb();
	}
}

/* =================================================================== */
/* description	        ::	share memory initialize */
/* function		::	shared_mem_init */
/* input		::	none */
/* output		::	none */
/* call			::	pmc_enable_periph_clk,ipc_enable_interrupt */
/* effect		::	Reg_In_CONTROL,Reg_Out_MODE,Acc_Out_I_T */
/* =================================================================== */
void configure_ipc( void )
{
	Metrology_Reg_In = ((__IO uint32_t *)mem_reg_in);
	Metrology_Reg_Out = ((__IO uint32_t *)mem_reg_out);
	Metrology_Acc_Out = ((__IO uint32_t *)mem_acc_out);
	Metrology_Har_Out = ((__IO uint32_t *)mem_har_out);

	/* Enable IPCs */
	ipc_enable(IPC0);
	ipc_enable(IPC1);

	/* ipc_set_user_handlers */
	ipc_set_handler(IPC0, IPC_INTERRUPT_SRC_IRQ20, ipc_init_irq_handler);
	ipc_set_handler(IPC0, IPC_INTERRUPT_SRC_IRQ16, ipc_status_handler);
	ipc_set_handler(IPC0, IPC_INTERRUPT_SRC_IRQ5, ipc_half_cycle_handler);
	ipc_set_handler(IPC0, IPC_INTERRUPT_SRC_IRQ4, ipc_full_cycle_irq_handler);
	ipc_set_handler(IPC0, IPC_INTERRUPT_SRC_IRQ0, ipc_integration_irq_handler);

	/* init cb event function */
	spf_ipc_integration_event_cb = NULL;

	/* ipc_enable_interrupt(IPC_CORE0, IPC_IRQ); */
	ipc_enable_interrupt( IPC0, IPC_INIT_IRQ | IPC_STATUS_IRQ | IPC_HALF_CYCLE_IRQ | IPC_FULL_CYCLE_IRQ | IPC_INTEGRATION_IRQ );
	NVIC_DisableIRQ( IPC0_IRQn );
	NVIC_ClearPendingIRQ( IPC0_IRQn );
	NVIC_EnableIRQ( IPC0_IRQn );
}

void ipc_integration_event_set_callback(void (*pf_ipc_event_cb)(void))
{
	spf_ipc_integration_event_cb = pf_ipc_event_cb;
}
