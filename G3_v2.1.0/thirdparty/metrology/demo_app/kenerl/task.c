/**
 * \file
 *
 * \brief TASK Process Handler.
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
#include "task.h"
#include "utils.h"
	
task_t VTask;
	
/**
 * \brief Init task queue
 */
void TaskInit(void)
{
	memset(&VTask, 0, sizeof(VTask));
}

/**
 * \brief Put task into task queue in interrupt state
 *
 * \return 1:success, 0=failure
 */
uint8_t TaskPutIntoQueueInterrupt(pf_task_t task)
{
	if (VTask.num_tasks >= TASK_NUM_MAX) {
		LOG_APP_DEMO_DEBUG(("TASK QUEUE: TASK_NUM_MAX is reached. Task discarded."));
		return 0;
	}

	VTask.Buff[VTask.put_idx++] = task;
	if (VTask.put_idx == TASK_NUM_MAX) {
		VTask.put_idx = 0;
	}
	
	VTask.num_tasks++;

	return 1;
}

/**
 * \brief Put task into task queue.
 *
 * \return 1:success, 0=failure
 */
uint8_t TaskPutIntoQueue(pf_task_t task)
{
	uint8_t i;
	
	__disable_irq();
	i = TaskPutIntoQueueInterrupt(task);
	__enable_irq();

	return (i);
}

/**
 * \brief Get task from task queue.
 *
 * \return 1 if Task has been executed, 0 otherwise
 */
uint8_t TaskRunFromQueue(void)
{
	uint8_t res = 0;
	
	if (VTask.Buff[VTask.get_idx]) {
		VTask.Buff[VTask.get_idx]();
		VTask.Buff[VTask.get_idx++] = NULL;
		VTask.num_tasks--;
		
		if (VTask.get_idx == TASK_NUM_MAX) {
			VTask.get_idx = 0;
		}
	
		res = 1;
	}

	return (res);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
