/**
 * \file
 *
 * \brief Meter Demo : Command module
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

#include <string.h>
#include <math.h>

#include "uart.h"
#include "usart.h"
#include "conf_uart_serial.h"
#include "conf_command_serial.h"
#include "conf_board.h"
#include "task.h"
#include "extmem.h"
#include "command.h"
#include "metrology.h"
#include "energy.h"
#include "history.h"
#include "demand.h"
#include "rtcproc.h"
#include "tou.h"
#include "event.h"
#include "utils.h"
#include "main.h"
#include "rstc.h"
#include "delay.h"
#include "dwdt.h"
#include "coproc.h"
#include "icm.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

command_t VCom;

static uint8_t spuc_pdc_buff[FRAME_MAX_SEND_LEN];
const char *del_cmd = "\b \b";

static com_port_data_t *com_har_ptr, *com_cal_ptr;

/**
 * \brief Send bytes through serial port using PDC.
 *
 * \param proc_id Comm Port Identifier.
 *
 * \return 1 when complete command should be processed, 0 otherwise
 */
static uint8_t _send_data(com_port_id_t proc_id)
{
	uint32_t timeout;
	com_port_data_t *com_ptr;
	Pdc *p_pdc;
	pdc_packet_t pdc_packet;

	VCom.lamptimer = 2;

	/* Waiting previous transfers */
	timeout = COMPROC_TXFER_TIMEOUT;
	if (proc_id == COMPROC_CONSOLE_ID) {
		while (!usart_is_tx_buf_empty(CONF_COMMAND_UART)) {
			if (!timeout--) {
				return false;
			}
		}
	} else if (proc_id == COMPROC_OPTO_ID) {
		while (!uart_is_tx_empty(CONF_OPTO_UART)) {
			if (!timeout--) {
				return false;
			}
		}
	} else {
		return false;
	}

	com_ptr = &VCom.comport[proc_id];
	p_pdc = com_ptr->pdc_base;

	/* Disable PDC */
	pdc_disable_transfer(p_pdc, PERIPH_PTCR_TXTDIS);

	/* Configure DMA channels */
	memcpy(spuc_pdc_buff, com_ptr->send_buff, com_ptr->send_len);
	pdc_packet.ul_addr = (uint32_t)spuc_pdc_buff;
	pdc_packet.ul_size = com_ptr->send_len;
	pdc_tx_init(p_pdc, &pdc_packet, NULL);

	/* Enable PDC */
	pdc_enable_transfer(p_pdc, PERIPH_PTCR_TXTEN);

	return true;
}

/**
 * \brief Send prompt to console port
 */
static void _send_console_prompt(void)
{
	//VCom.comport[COMPROC_CONSOLE_ID].send_len = sprintf((char *)VCom.comport[COMPROC_CONSOLE_ID].send_buff, "%s", "\r\n>");
	//_send_data(COMPROC_CONSOLE_ID);
}

/**
 * \brief Compare Secure Password
 *
 * \param ptr Pointer to data first address
 *
 * \return true:success,false:failure
 *
 */
static bool _check_sec_pwd(uint8_t *ptr)
{
	if ((*ptr != '[') || (*(ptr + 4) != ']')) {
		return false;
	}

	if (memcmp(ptr+1, COMPROC_SEC_PWD, 3) != 0) {
		return false;
	}

	return true;
}

/**
 * \brief Get Value from the terminal buffer
 *
 * \param pul_dst  Pointer to data where store the value
 * The returned value is an integer, corresponding to the parameter value multiplied by 10000
 * (example: returned value is 10000 if the parameter=1; or 12456 if the parameter is 1.2456)
 * \param puc_src  Pointer to the source terminal buffer
 * \param len      Length of the source terminal data
 *
 * \return true:success, false:format error
 *
 */
static bool _get_value_terminal(uint32_t *pul_dst, char *puc_src, uint8_t len)
{
	uint32_t ul_value = 0;
	uint8_t isdecp = 0;
	uint8_t i;

	for (i = 0; i < len; i++, puc_src++) {
		if (((*puc_src) >= '0') && ((*puc_src) <= '9')) {
			if (isdecp > 0) {
				ul_value += (*puc_src - 0x30) * ((uint32_t)pow(10, len - i));
			} else {
				ul_value += (*puc_src - 0x30) * ((uint32_t)pow(10, len - 1 - i));
			}
		} else if (*puc_src == '.') {
			/* Decimal Pointer shouldn't be in the first/last position */
			if ((i == 0) || (i == (len -1))) {
				return false;
			}
			isdecp = i;
		} else {
			return false;
		}
	}

	if (isdecp > 0) {
		ul_value /= 10;
		ul_value *= ((uint32_t)pow(10, 5 + isdecp - len));
	} else {
		ul_value *= 10000;
	}

	*pul_dst = ul_value;

	return true;
}

/**
 * \brief Find string parameters in the terminal buffer and get its value
 *
 * \param pul_dst    Pointer to data where store the value.
 * The returned value is an integer, corresponding to the parameter value multiplied by 10000
 * (example: returned value is 10000 if the parameter=1; or 12456 if the parameter is 1.2456)
 * \param puc_param  Pointer to the parameter to find
 * \param puc_src    Pointer to the source terminal buffer
 *
 * \return true:success, false:format error
 */
static bool _get_param_terminal(uint32_t *pul_dst, char *puc_param, uint8_t *puc_src)
{
	char *p1;
	char *p2;

	/* Set Start Pointer */
	p1 = strstr((const char *)puc_src, (const char *)puc_param);
	if (p1) {
		p1 = strstr((const char *)p1, (const char *)"=");
	}
	/* Set End Pointer */
	p2 = strstr((const char *)p1, (const char *)",");
	if (p2 == 0) {
		p2 = strstr((const char *)p1, (const char *)")");
	}

	/* Get Value */
	if ((p2 - p1) > 1 ) {
		p1++;
		if (!_get_value_terminal(pul_dst, p1, p2-p1)) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

/**
 * \brief Callback Function to process Harmonics data
 *
 * \param p_har_calc Pointer to harmoinc data
 */
static void _har_callback(metrology_har_t *p_har_calc)
{
	double *har_data;
	uint8_t uc_idx;
	uint8_t reg_idx = 0;

	har_data = &p_har_calc->Irms_A_m;

	com_har_ptr->send_len = sprintf((char *)com_har_ptr->send_buff,
					"The calculated harmonic Irms/Vrms:\r\n");
	_send_data(COMPROC_CONSOLE_ID);

	/* Send all HARMONIC registers */
	for (uc_idx = 0; uc_idx < DSP_HRR_REG_NUM; uc_idx += 3) {
          if (DSP_HRR_REG_NUM-uc_idx>2) {
		com_har_ptr->send_len = sprintf((char *)com_har_ptr->send_buff,
						"%-19s%-19s%-19s\r\n%-19.3f%-19.3f%-19.3f\r\n",
						(char *)dsp_hrr_header[reg_idx],
						(char *)dsp_hrr_header[reg_idx + 1],
						(char *)dsp_hrr_header[reg_idx + 2],
						*har_data, *(har_data + 1), *(har_data + 2));
          } else if (DSP_HRR_REG_NUM-uc_idx>1) {
                com_har_ptr->send_len = sprintf((char *)com_har_ptr->send_buff,
						"%-19s%-19s\r\n%-19.3f%-19.3f\r\n",
						(char *)dsp_hrr_header[reg_idx],
						(char *)dsp_hrr_header[reg_idx + 1],
						*har_data, *(har_data + 1));
          } else if (DSP_HRR_REG_NUM-uc_idx>0) {
                com_har_ptr->send_len = sprintf((char *)com_har_ptr->send_buff,
						"%-19s\r\n%-19.3f\r\n",
						(char *)dsp_hrr_header[reg_idx],
						*har_data);
          }

		_send_data(COMPROC_CONSOLE_ID);

		reg_idx += 3;
		har_data += 3;
	}
}

/**
 * \brief Callback Function to signal the end of the Calibration process.
 *
 * \param p_har_calc Pointer to harmoinc data
 */
static void _cal_callback(uint8_t error)
{
	if (error) {
		com_cal_ptr->send_len = sprintf((char *)com_cal_ptr->send_buff,
					"Calibration failure. Ensure a CNF command was sent before calibration!\r\n");
	} else {
		com_cal_ptr->send_len = sprintf((char *)com_cal_ptr->send_buff,
					"Calibrating Done!\r\n");
	}
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_buf(com_port_data_t *com_ptr)
{
	int32_t *data_ptr;
	uint32_t num_samples, start_sample, end_sample, i;
	uint8_t *puc_rcv_data;
	char *p1;
	char *p2;
	uint8_t buf_idx;
	uint32_t sector_idx;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];
	/* Check Paramereter: Data sector (BUF[xxx]) */
  	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");

	if ((p1 == NULL) || (p2 == NULL)) {
		/* Send all buffer */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "Waveform Capture Data...\r\n");
		_send_data(COMPROC_CONSOLE_ID);

		/* Set Data pointer to Capture Buffer */
		num_samples = MetrologyGetCaptureData(&data_ptr);

		for (i = 0; i < num_samples; i++, data_ptr++) {
			com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%08X\r\n", *data_ptr);
			_send_data(COMPROC_CONSOLE_ID);
			dwdt_restart(DWDT, WDT0_ID);
		}

		/* Format compatibility with older versions */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "\r\n");
		_send_data(COMPROC_CONSOLE_ID);

	} else if (((p2 - p1) < 2) || ((p2 - p1) > 4)) {
		/* Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "Unsupported command...\r\n");
		_send_data(COMPROC_CONSOLE_ID);
	} else {
		sector_idx = *(p1 + 1) - 0x30;
		if ((p2 - p1) > 2) {
			/* Parameter is in 2 digits */
			sector_idx *= 10;
			sector_idx += *(p1 + 2) - 0x30;
		}
		if ((p2 - p1) > 3) {
			/* Parameter is in 3 digits */
			sector_idx *= 10;
			sector_idx += *(p1 + 3) - 0x30;
		}

		/* Send a sector of 512 samples */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "Waveform Capture Data...\r\n");
		_send_data(COMPROC_CONSOLE_ID);
		/* Set Data pointer to Capture Buffer */
		num_samples = MetrologyGetCaptureData(&data_ptr);
		start_sample= sector_idx*512;
		end_sample= start_sample + 512;
		if (num_samples < end_sample) {
			end_sample=num_samples;
		}
		if (start_sample <= num_samples) {
			for (i = start_sample; i < end_sample; i++, data_ptr++) {
				com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%08X\r\n", *(data_ptr+start_sample));
				_send_data(COMPROC_CONSOLE_ID);
			}
		}
	}
}

static void _process_cmd_cal(com_port_data_t *com_ptr)
{
	uint8_t *puc_rcv_data;
	uint8_t buf_idx;
	bool format_err = false;
	uint8_t phase;
	uint32_t ux, ix, ax;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	phase = *(puc_rcv_data + 1);

	/* Get first Ux value */
	if (!_get_param_terminal(&ux, "U", puc_rcv_data)) {
		format_err = true;
	}
	ux /= 10;
	/* Get first Ix value */
	if (!_get_param_terminal(&ix, "I", puc_rcv_data)) {
		format_err = true;
	}
	/* Get first Ax value */
	if (!_get_param_terminal(&ax, "AA", puc_rcv_data)) {
		if (!_get_param_terminal(&ax, "AB", puc_rcv_data)) {
			if (!_get_param_terminal(&ax, "AC", puc_rcv_data)) {
				format_err = true;
			}
		}
	}
	ax /= 10;

	if (ax >= 360000) {		/* Valid range: from 0 to 360 degrees */
		format_err = true;
	}

	if (phase == 'A') {
		VCalibration.aim_va = ux;
		VCalibration.aim_ia = ix;
		VCalibration.angle_a = ax;
		VCalibration.line_id = Ph_A;
	} else if (phase == 'B') {
		VCalibration.aim_vb = ux;
		VCalibration.aim_ib = ix;
		VCalibration.angle_b = ax;
		VCalibration.line_id = Ph_B;
	} else if (phase == 'C') {
		VCalibration.aim_vc = ux;
		VCalibration.aim_ic = ix;
		VCalibration.angle_c = ax;
		VCalibration.line_id = Ph_C;
	} else if (phase == 'T') {
		VCalibration.aim_va = ux;
		VCalibration.aim_ia = ix;
		VCalibration.angle_a = ax;
		VCalibration.line_id = Ph_T;

		/* Get Ub value */
		if (!_get_param_terminal(&VCalibration.aim_vb, "UB", puc_rcv_data)) {
			format_err = true;
		}
		VCalibration.aim_vb /= 10;
		/* Get Ib value */
		if (!_get_param_terminal(&VCalibration.aim_ib, "IB", puc_rcv_data)) {
			format_err = true;
		}
		/* Get Ab value */
		if (!_get_param_terminal(&VCalibration.angle_b, "AB", puc_rcv_data)) {
			format_err = true;
		}
		VCalibration.angle_b /= 10;
		if (VCalibration.angle_b >= 360000) {		/* Valid range: from 0 to 360 degrees */
			format_err = true;
		}

		/* Get Uc value */
		if (!_get_param_terminal(&VCalibration.aim_vc, "UC", puc_rcv_data)) {
			format_err = true;
		}
		VCalibration.aim_vc /= 10;
		/* Get Ic value */
		if (!_get_param_terminal(&VCalibration.aim_ic, "IC", puc_rcv_data)) {
			format_err = true;
		}
		/* Get Ac value */
		if (!_get_param_terminal(&VCalibration.angle_c, "AC", puc_rcv_data)) {
			format_err = true;
		}
		VCalibration.angle_c /= 10;
		if (VCalibration.angle_c >= 360000) {		/* Valid range: from 0 to 360 degrees */
			format_err = true;
		}

	} else {
		format_err = true;
	}

	if (format_err) {
		/* Format ERROR: Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		/* Set comm for the calibration callback function */
		com_cal_ptr = com_ptr;
		/* Launch Calibration Task of the metrology module */
		VCalibration.dsp_update_num = 0;
		TaskPutIntoQueue(MetrologyCalibMeter);
		/* Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "It is calibrating...\r\n");
	}

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_cnf(com_port_data_t *com_ptr)
{
	uint8_t *puc_rcv_data;
	uint8_t buf_idx;
	bool format_err = false;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Get Meter Constant */
	if (!_get_param_terminal(&VCalibration.mc, "MC", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.mc /= 10000;

	/* Get Sensor Type */
	if (!_get_param_terminal(&VCalibration.sensortype, "ST", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.sensortype /= 10000;
	/* Check Sensor Type Value */
	if (VCalibration.sensortype > (SENSOR_NUM_TYPE)) {
		format_err = true;
	}

	/* Get Frequency */
	if (!_get_param_terminal(&VCalibration.freq, "F", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.freq /= 100;

	/* Get Gain */
	if (!_get_param_terminal(&VCalibration.gain_i, "G", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.gain_i /= 10000;

	/* Get Transformer ratio */
	if (!_get_param_terminal(&VCalibration.k_i, "TR", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.k_i /= 10;

	/* Get Resister Load */
	if (!_get_param_terminal(&VCalibration.rl, "RL", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.rl /= 100;

	/* Get Voltage divider ratio */
	if (!_get_param_terminal(&VCalibration.k_u, "KU", puc_rcv_data)) {
		format_err = true;
	}
	VCalibration.k_u /= 10;

	if (format_err) {
		/* Format ERROR: Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		VCalibration.dsp_update_num = 0;
		/* Launch Calibration Task of the metrology module */
		MetrologyCalibMeterInit();
		/* Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Configure Meter is ok !\r\n");
	}

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_dar(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	uint8_t reg_idx = 0;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	if (p1 == NULL) {
		uint8_t uc_idx;

		/* Send all ACC registers */
		for (uc_idx = 0; uc_idx < DSP_ACC_REG_NUM; uc_idx += 4) {
			rsp_len = sprintf((char *)com_ptr->send_buff, "%-19s%-19s%-19s%-19s\r\n%-19jX%-19jX%-19jX%-19jX\r\n",
					  (char *)dsp_acc_header[reg_idx], (char *)dsp_acc_header[reg_idx + 1],
					  (char *)dsp_acc_header[reg_idx + 2], (char *)dsp_acc_header[reg_idx + 3],
					  *dsp_acc_str[reg_idx], *dsp_acc_str[reg_idx + 1], *dsp_acc_str[reg_idx + 2],
					  *dsp_acc_str[reg_idx + 3]);

			/* Adjust the last line in case of not full filled */
			if ((reg_idx + 4) > DSP_ACC_REG_NUM) {
				/* Get over parameters */
				uint8_t over_prm = reg_idx + 4 - DSP_ACC_REG_NUM;
				/* Set source pointer of data to move */
				p1 = (char *)(com_ptr->send_buff + (4 * 19));
				/* Set destiny pointer to move */
				p2 = (char *)(com_ptr->send_buff + ((4 - over_prm) * 19));
				/* Move data */
				memcpy(p2, p1, (4 - over_prm) * 19);
				/* Insert CR/LF */
				rsp_len = (4 - over_prm) * 38 + 2;
				com_ptr->send_buff[rsp_len++] = 0x0D;
				com_ptr->send_buff[rsp_len++] = 0x0A;
			}

			com_ptr->send_len = rsp_len;
			_send_data(COMPROC_CONSOLE_ID);

			reg_idx += 4;
		}
	} else {
		p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
		if ( ((p2 - p1) > 3) || ((p2 - p1) < 2) ) {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
		} else {
			/* Get Register Index */
			reg_idx = *(p1 + 1) - 0x30;
			if ((p2 - p1) == 3) {
				reg_idx *= 10;
				reg_idx += *(p1 + 2) - 0x30;
			}

			/* Check Register Index */
			if (reg_idx < DSP_ACC_REG_NUM) {
				/* Send register content */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s\r\n%jX\r\n",
						  (char *)dsp_acc_header[reg_idx], *dsp_acc_str[reg_idx]);
			} else {
				/* Format ERROR: Send Response */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
			}
		}

		com_ptr->send_len = rsp_len;

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}
}

static void _process_cmd_dcb(com_port_data_t *com_ptr)
{
	(void)com_ptr;

	/* Set Backup Mode */
	Vsys.mode = BACKUP_MODE;
}

static void _process_cmd_dcd(com_port_data_t *com_ptr)
{
	char *puc_send_data;

	/* Load Metrolgoy default values */
	MetrologyLoadDefault();
	/* Refresh Control in Shared Memory region */
	MetrologyRefreshCrtl();
	/* Send Response */
	puc_send_data = (char *)com_ptr->send_buff;
	com_ptr->send_len = sprintf(puc_send_data, "%s", "Load Default is ok !\r\n");

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_rst(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Reset Command is ok !\r\n");
		_send_data(COMPROC_CONSOLE_ID);
		/* Time to send the message */
		delay_ms(10);
		/* SW Reset */
		rstc_start_software_reset(RSTC);
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}


static void _process_cmd_rld(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
                /* Preliminary actions before calling CoprocInit */
                /* 1) Disable IPC Interrupt */
                NVIC_DisableIRQ(IPC1_IRQn);
                NVIC_ClearPendingIRQ(IPC1_IRQn);
                /* 2) Disable ICM */
                icm_reset(ICM);
                NVIC_DisableIRQ(ICM_IRQn);
                NVIC_ClearPendingIRQ(ICM_IRQn);
#if BOARD == PIC32CXMTSH_DB
                /* 3) Disable EMAFE clock */
                sysclk_disable_peripheral_clock(ID_EMAFE);
                /* 4) Put ATSense into hardware RESET */
                macro_pio_config(PIOD, (1u<<30), (PIO_CFGR_FUNC_GPIO | PIO_CFGR_DIR_OUTPUT | PIO_CFGR_SLEWRATE_SLOW));
                PIOD->PIO_CODR = (1u<<30);
#endif
                /* 5) Disable ATSense clock */
                pmc_disable_pck(PMC_PCK_2);

                /* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Reload metrology command received!\r\n");
		_send_data(COMPROC_CONSOLE_ID);
		/* Time to send the message */
		delay_ms(10);

		/* Reload metrology */
		CoprocInit(ICM_MONITORING, RESET_TYPE_HARD);
                com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Reloaded core-1 image!\r\n");
                _send_data(COMPROC_CONSOLE_ID);
                /* Time to send the message */
                delay_ms(10);

		if (!MetrologyInit()) {
			LOG_APP_DEMO_WDG0_DEBUG(("\r\n\t-I- Coproc INIT...ERROR....reseting\r\n"));
                        com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Metrology Initializarion failing. Reseting!\r\n");
                        _send_data(COMPROC_CONSOLE_ID);
			delay_ms(100);
			rstc_start_software_reset(RSTC);
		}
		LOG_APP_DEMO_WDG0_DEBUG(("\r\n\t-I- Coproc INIT...OK\r\n"));
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Metrology lib reloaded!\r\n");
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_dcm(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	bool b_find_next;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

//	DCM( 31:1E7CA0AF; 32:1249DE60; 33:1E926E77; 34:11752E95; 35:1A6EEE5B; 36:10B25ED3; 37:207D2F14 )

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)":");
	if (p1 == NULL) {
		/* Format ERROR: Send Response */
		rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		uint32_t reg_value;
		uint8_t reg_idx;
		uint8_t value_len, i;
		bool format_err, validation_err;

		b_find_next = true;
		format_err = false;
		validation_err = false;

		while (b_find_next) {
			p2 = strstr((const char *)p1, (const char *)";");
			if (p2 == NULL) {
				p2 = strstr((const char *)p1, (const char *)")");
				/* Last parameter */
				b_find_next = false;
				if (p2 == NULL) {
					/* Format ERROR: Send Response */
					format_err = true;
				}
			}

			/* Get Register Index */
			reg_idx = *(p1 - 2) - 0x30;
			reg_idx *= 10;
			reg_idx += *(p1 - 1) - 0x30;

			/* Get Register Value */
			p1++; /* Skip ":" */
			value_len = p2 - p1;
			if (value_len > 8) {
				value_len = 8;
			}
			reg_value = 0;
			for (i = 0; i < value_len; i++, p1++) {
				if (((*p1) >= '0') && ((*p1) <= '9')) {
					reg_value <<= 4;
					reg_value += (*p1 - 0x30);
				} else if (((*p1) >= 'A') && ((*p1) <= 'F')) {
					reg_value <<= 4;
					reg_value += (*p1 - 0x37);
				} else if ((*p1) == ' ') {
					break;
				} else {
					format_err = true;
					break;
				}
			}

			if (format_err) {
				/* Stop search */
				b_find_next = false;
			} else {
				/* Check Metrology DSP control register */
				if (reg_idx < DSP_CTRL_REG_NUM) {
					uint32_t *puc_reg;

					/* Data validation registers 46 and 47 (CAPT_BUFF_SIZE and CAPTURE_ADDR) */
					if (reg_idx == MET_CAPTURE_BUFF_LENGTH_OFFSET || reg_idx == MET_CAPTURE_ADDR_OFFSET) {
					    uint32_t temp_address, temp_size;
					    if (reg_idx == MET_CAPTURE_BUFF_LENGTH_OFFSET) {
						temp_size = reg_value;
						temp_address = VMetrology.DSP_CTRL.CAPTURE_ADDR;
					    } else {
						temp_size = VMetrology.DSP_CTRL.CAPTURE_BUFF_SIZE.WORD;
						temp_address = reg_value;
					    }
					    if (temp_address < ( uint32_t )(&VCapture_Buff[0])) validation_err = true;
					    if ((temp_address + temp_size*4) > (MET_CAPTURE_BUFF_LEN*4 + ( uint32_t )(&VCapture_Buff[0]))) validation_err = true;
					}

					if (validation_err) {
						/* Stop search */
						b_find_next = false;

					} else {
						/* Update Metrology DSP control register */
						puc_reg = (uint32_t *)dsp_ctrl_str[reg_idx];
						*puc_reg = reg_value;
					}


				} else {
					/* Stop search */
					b_find_next = false;
					/* Format ERROR: Send Response */
					format_err = true;
				}
			}

			if (b_find_next) {
				/* Find next parameter */
				p1 = strstr((const char *)p1, (const char *)":");
			}
		}

		if (format_err) {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Set is Failure\r\n");
		} else if (validation_err) {
			/* Validation ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Rejected (capture buffer error)\r\n");
		} else {
			/* Refresh control register data in shared memory */
			MetrologyRefreshCrtl();
			/* Build Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Set is Ok\r\n");
		}
	}

	com_ptr->send_len = rsp_len;

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_dcr(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	uint8_t reg_idx = 0;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	if (p1 == NULL) {
		uint8_t uc_idx;

		/* Send all CONTROL registers */
		for (uc_idx = 0; uc_idx < DSP_CTRL_REG_NUM; uc_idx += 4) {
			rsp_len = sprintf((char *)com_ptr->send_buff, "%-19s%-19s%-19s%-19s\r\n%-19X%-19X%-19X%-19X\r\n",
					  (char *)dsp_ctrl_header[reg_idx], (char *)dsp_ctrl_header[reg_idx + 1],
					  (char *)dsp_ctrl_header[reg_idx + 2], (char *)dsp_ctrl_header[reg_idx + 3],
					  *dsp_ctrl_str[reg_idx], *dsp_ctrl_str[reg_idx + 1], *dsp_ctrl_str[reg_idx + 2],
					  *dsp_ctrl_str[reg_idx + 3]);

			/* Adjust the last line in case of not full filled */
			if ((reg_idx + 4) > DSP_CTRL_REG_NUM) {
				/* Get over parameters */
				uint8_t over_prm = reg_idx + 4 - DSP_CTRL_REG_NUM;
				/* Set source pointer of data to move */
				p1 = (char *)(com_ptr->send_buff + (4 * 19));
				/* Set destiny pointer to move */
				p2 = (char *)(com_ptr->send_buff + ((4 - over_prm) * 19));
				/* Move data */
				memcpy(p2, p1, (4 - over_prm) * 19);
				/* Insert CR/LF */
				rsp_len = (4 - over_prm) * 38 + 2;
				com_ptr->send_buff[rsp_len++] = 0x0D;
				com_ptr->send_buff[rsp_len++] = 0x0A;
			}

			com_ptr->send_len = rsp_len;
			_send_data(COMPROC_CONSOLE_ID);

			reg_idx += 4;
		}
	} else {
		p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
		if ( ((p2 - p1) > 3) || ((p2 - p1) < 2) ) {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
		} else {
			/* Get Register Index */
			reg_idx = *(p1 + 1) - 0x30;
			if ((p2 - p1) == 3) {
				reg_idx *= 10;
				reg_idx += *(p1 + 2) - 0x30;
			}

			/* Check Register Index */
			if (reg_idx < DSP_CTRL_REG_NUM) {
				/* Send register content */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s\r\n%X\r\n",
						  (char *)dsp_ctrl_header[reg_idx], *(uint32_t *)dsp_ctrl_str[reg_idx]);
			} else {
				/* Format ERROR: Send Response */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
			}
		}

		com_ptr->send_len = rsp_len;

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}
}

static void _process_cmd_dcs(com_port_data_t *com_ptr)
{
	if (MetrologyUpdateExtMem()) {
		/* Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Save data is ok !\r\n");
	} else {
		/* Format ERROR: Send Response */
		com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Error saving data !\r\n");
	}

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_dcw(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
	if ( ((p2 - p1) > 3) || ((p2 - p1) < 2) ) {
		/* Format ERROR: Send Response */
		rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		uint32_t reg_value = 0;
		uint8_t reg_idx = 0;
		uint8_t value_len, i;
		bool format_err, validation_err;

		/* Get Register Index */
		reg_idx = *(p1 + 1) - 0x30;
		if ((p2 - p1) == 3) {
			reg_idx *= 10;
			reg_idx += *(p1 + 2) - 0x30;
		}
		p1 = strstr((const char *)puc_rcv_data, (const char *)"(");
		p2 = strstr((const char *)puc_rcv_data, (const char *)")");

		/* Get Register Value */
		p1++;
		value_len = p2 - p1;
		format_err = false;
		validation_err = false;
		for (i = 0; i < value_len; i++, p1++) {
			if (((*p1) >= '0') && ((*p1) <= '9')) {
				reg_value <<= 4;
				reg_value += (*p1 - 0x30);
			} else if (((*p1) >= 'A') && ((*p1) <= 'F')) {
				reg_value <<= 4;
				reg_value += (*p1 - 0x37);
			} else {
				format_err = true;
				break;
			}
		}

		if (format_err) {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Set ST_CTRL is Failure\r\n");
		} else {
			/* Check Metrology DSP control register */
			if (reg_idx < DSP_CTRL_REG_NUM) {
				uint32_t *puc_reg;

				/* Data validation registers 46 and 47 (CAPT_BUFF_SIZE and CAPTURE_ADDR) */
				if (reg_idx == MET_CAPTURE_BUFF_LENGTH_OFFSET || reg_idx == MET_CAPTURE_ADDR_OFFSET) {
				    uint32_t temp_address, temp_size;
				    if (reg_idx == MET_CAPTURE_BUFF_LENGTH_OFFSET) {
					temp_size = reg_value;
					temp_address = VMetrology.DSP_CTRL.CAPTURE_ADDR;
				    } else {
					temp_size = VMetrology.DSP_CTRL.CAPTURE_BUFF_SIZE.WORD;
					temp_address = reg_value;
				    }
				    if (temp_address < ( uint32_t )(&VCapture_Buff[0])) validation_err = true;
				    if ((temp_address + temp_size*4) > (MET_CAPTURE_BUFF_LEN*4 + ( uint32_t )(&VCapture_Buff[0]))) validation_err = true;
				}

				if (validation_err) {
					/* Validation ERROR: Send Response */
					rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Rejected (capture buffer error)\r\n");
				} else {
					/* Update Metrology DSP control register */
					puc_reg = (uint32_t *)dsp_ctrl_str[reg_idx];
					*puc_reg = reg_value;

					/* Refresh control register data in shared memory */
					MetrologyRefreshCrtl();

					rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Set ST_CTRL is Ok\r\n");
				}
			} else {
				/* Format ERROR: Send Response */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Set ST_CTRL is Failure\r\n");
			}
		}
	}

	com_ptr->send_len = rsp_len;

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_dsr(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	uint8_t reg_idx = 0;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	if (p1 == NULL) {
		uint8_t uc_idx;

		/* Send all STATUS registers */
		for (uc_idx = 0; uc_idx < DSP_ST_REG_NUM; uc_idx += 4) {
			rsp_len = sprintf((char *)com_ptr->send_buff, "%-19s%-19s%-19s%-19s\r\n%-19X%-19X%-19X%-19X\r\n",
					  (char *)dsp_st_header[reg_idx], (char *)dsp_st_header[reg_idx + 1],
					  (char *)dsp_st_header[reg_idx + 2], (char *)dsp_st_header[reg_idx + 3],
					  *dsp_st_str[reg_idx], *dsp_st_str[reg_idx + 1], *dsp_st_str[reg_idx + 2],
					  *dsp_st_str[reg_idx + 3]);

			/* Adjust the last line in case of not full filled */
			if ((reg_idx + 4) > DSP_ST_REG_NUM) {
				/* Get over parameters */
				uint8_t over_prm = reg_idx + 4 - DSP_ST_REG_NUM;
				/* Set source pointer of data to move */
				p1 = (char *)(com_ptr->send_buff + (4 * 19));
				/* Set destiny pointer to move */
				p2 = (char *)(com_ptr->send_buff + ((4 - over_prm) * 19));
				/* Move data */
				memcpy(p2, p1, (4 - over_prm) * 19);
				/* Insert CR/LF */
				rsp_len = (4 - over_prm) * 38 + 2;
				com_ptr->send_buff[rsp_len++] = 0x0D;
				com_ptr->send_buff[rsp_len++] = 0x0A;
			}

			com_ptr->send_len = rsp_len;
			_send_data(COMPROC_CONSOLE_ID);

			reg_idx += 4;
		}
	} else {
		p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
		if ( ((p2 - p1) > 3) || ((p2 - p1) < 2) ) {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
		} else {
			/* Get Register Index */
			reg_idx = *(p1 + 1) - 0x30;
			if ((p2 - p1) == 3) {
				reg_idx *= 10;
				reg_idx += *(p1 + 2) - 0x30;
			}

			/* Check Register Index */
			if (reg_idx < DSP_ST_REG_NUM) {
				/* Send register content */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s\r\n%X\r\n",
						  (char *)dsp_st_header[reg_idx], *(uint32_t *)dsp_st_str[reg_idx]);
			} else {
				/* Format ERROR: Send Response */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
			}
		}

		com_ptr->send_len = rsp_len;

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}
}

static void _process_cmd_enc(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		/* Clear Energy */
		EnergyClear();
		/* Clear History (energy storage for the last year */
		HistoryClearAll();
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Clear Energy is ok !\r\n");
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_enr(com_port_data_t *com_ptr)
{
	uint8_t *puc_rcv_data;
	char *puc_send_data;
	char *p1;
	char *p2;
	energy_t energy_data = {0};
	uint32_t tou_total;
	uint8_t month_idx, month_offset, buf_idx;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];
	/* Check Paramereters: Month */
  	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");

	if ((p1 == NULL) || (p2 == NULL)) {
		/* No parameter. Send all stored months */
		for (month_offset=0; month_offset<12; month_offset++) {
			if (month_offset == 0) {
				/* Get Current Energy data */
				energy_data = VEnergy;
			} else {
				/* Get Energy from History data */
				month_idx = (VRTC.date.month - month_offset);
				while (month_idx > 12 || month_idx==0) {
					/* Overflow in month index */
					month_idx += 12;
				}
				HistoryGetEnergyMonth(&energy_data, month_idx);
			}

			/* Get TOU total energy */
			tou_total = energy_data.tou1_acc + energy_data.tou2_acc + energy_data.tou3_acc + energy_data.tou4_acc;

			/* Send Response */
			puc_send_data = (char *)com_ptr->send_buff;
			com_ptr->send_len = sprintf(puc_send_data, "Last %d Month Energy is:\r\nTT=%.2fkWh T1=%.2fkWh T2=%.2fkWh T3=%.2fkWh T4=%.2fkWh\r\n",
						    month_offset, (float)tou_total/10000000, (float)energy_data.tou1_acc/10000000, (float)energy_data.tou2_acc/10000000,
						    (float)energy_data.tou3_acc/10000000, (float)energy_data.tou4_acc/10000000);

			/* Send response to serial com */
			_send_data(COMPROC_CONSOLE_ID);
		}
	} else if (((p2 - p1) < 2) || ((p2 - p1) > 3)) {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Unsupported Command !\r\n");
		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	} else {
		month_offset = *(p1 + 1) - 0x30;
		if ((p2 - p1) > 2) {
			/* Month offset Parameter is in 2 digits */
			month_offset *= 10;
			month_offset += *(p1 + 2) - 0x30;
		}

		if (month_offset == 0) {
			/* Get Current Energy data */
			energy_data = VEnergy;
		} else {
			/* Get Energy from History data */
			month_idx = (VRTC.date.month - month_offset);
			while (month_idx > 12 || month_idx==0) {
				/* Overflow in month index */
				month_idx += 12;
			}
			HistoryGetEnergyMonth(&energy_data, month_idx);
		}

		/* Get TOU total energy */
		tou_total = energy_data.tou1_acc + energy_data.tou2_acc + energy_data.tou3_acc + energy_data.tou4_acc;

		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "Last %d Month Energy is:\r\nTT=%.2fkWh T1=%.2fkWh T2=%.2fkWh T3=%.2fkWh T4=%.2fkWh\r\n",
					    month_offset, (float)tou_total/10000000, (float)energy_data.tou1_acc/10000000, (float)energy_data.tou2_acc/10000000,
					    (float)energy_data.tou3_acc/10000000, (float)energy_data.tou4_acc/10000000);

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}


}

static void _process_cmd_evec(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][4];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		/* Clear Events */
		EventClear();
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Clear All Event is ok !\r\n");
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_ever(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len = 0;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	uint8_t last_num_times;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
	if ((p2 - p1) != 3) {
		/* Format ERROR: Send Response */
		rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		event_data_t ev_data;
		event_id_t ev_id = (event_id_t)0;

		/* Get Last number of times */
		p2 = strstr((const char *)(p2 + 1), (const char *)"]");
		last_num_times = CHARTODEC(*(p1 + 5));
		if ((p2 - p1) == 7) {
			last_num_times *= 10;
			last_num_times += CHARTODEC(*(p1 + 6));
		}

		/* Get Event Type */
		p1++;
		if (*p1 == 'U') {
			/* SAG Event Type */
			p1++;
			/* Get Event Phase */
			if (*p1 == 'A') {
				rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Times Ua Sag is :\r\n", last_num_times);
				ev_id = SAG_UA_EVENT;
			} else if (*p1 == 'B') {
				rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Times Ub Sag is :\r\n", last_num_times);
				ev_id = SAG_UB_EVENT;
			} else if (*p1 == 'C') {
				rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Times Uc Sag is :\r\n", last_num_times);
				ev_id = SAG_UC_EVENT;
			}
		} else if (*p1 == 'P') {
			/* P Reverse Event Type */
			p1++;
			/* Get Event Phase */
			if (*p1 == 'A') {
				rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Times Pa reverse is :\r\n", last_num_times);
				ev_id = POW_UA_EVENT;
			} else if (*p1 == 'B') {
				rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Times Pb reverse is :\r\n", last_num_times);
				ev_id = POW_UB_EVENT;
			} else if (*p1 == 'C') {
				rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Times Pc reverse is :\r\n", last_num_times);
				ev_id = POW_UC_EVENT;
			}

		}

		/* Get Event Info */
		if (EventGetData(&ev_data, ev_id, last_num_times)) {
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len],
					   "Total Num=%d start time is %02u-%02u %02u:%02u end time is %02u-%02u %02u:%02u\r\n", ev_data.counter,
					   ev_data.date_start.month, ev_data.date_start.day, ev_data.time_start.hour, ev_data.time_start.minute,
					   ev_data.date_end.month, ev_data.date_end.day, ev_data.time_end.hour, ev_data.time_end.minute);
		} else {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Unsupported Command !\r\n");
		}
	}

	com_ptr->send_len = rsp_len;

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_har(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	uint8_t reg_idx = 0;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	if (p1 == NULL) {
		uint8_t uc_idx;

		/* Send all HAR registers */
		for (uc_idx = 0; uc_idx < DSP_HAR_REG_NUM; uc_idx += 4) {
			rsp_len = sprintf((char *)com_ptr->send_buff, "%-19s%-19s%-19s%-19s\r\n%-19X%-19X%-19X%-19X\r\n",
					  (char *)dsp_har_header[reg_idx], (char *)dsp_har_header[reg_idx + 1],
					  (char *)dsp_har_header[reg_idx + 2], (char *)dsp_har_header[reg_idx + 3],
					  *dsp_har_str[reg_idx], *dsp_har_str[reg_idx + 1], *dsp_har_str[reg_idx + 2],
					  *dsp_har_str[reg_idx + 3]);

			/* Adjust the last line in case of not full filled */
			if ((reg_idx + 4) > DSP_HAR_REG_NUM) {
				/* Get over parameters */
				uint8_t over_prm = reg_idx + 4 - DSP_HAR_REG_NUM;
				/* Set source pointer of data to move */
				p1 = (char *)(com_ptr->send_buff + (4 * 19));
				/* Set destiny pointer to move */
				p2 = (char *)(com_ptr->send_buff + ((4 - over_prm) * 19));
				/* Move data */
				memcpy(p2, p1, (4 - over_prm) * 19);
				/* Insert CR/LF */
				rsp_len = (4 - over_prm) * 38 + 2;
				com_ptr->send_buff[rsp_len++] = 0x0D;
				com_ptr->send_buff[rsp_len++] = 0x0A;
			}

			com_ptr->send_len = rsp_len;
			_send_data(COMPROC_CONSOLE_ID);

			reg_idx += 4;
		}
	} else {
		p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
		if ( ((p2 - p1) > 3) || ((p2 - p1) < 2) ) {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
		} else {
			/* Get Register Index */
			reg_idx = *(p1 + 1) - 0x30;
			if ((p2 - p1) == 3) {
				reg_idx *= 10;
				reg_idx += *(p1 + 2) - 0x30;
			}

			/* Check Register Index */
			if (reg_idx < DSP_HAR_REG_NUM) {
				/* Send register content */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s\r\n%X\r\n",
						  (char *)dsp_har_header[reg_idx], *(uint32_t *)dsp_har_str[reg_idx]);
			} else {
				/* Format ERROR: Send Response */
				rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
			}
		}

		com_ptr->send_len = rsp_len;

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}
}

static void _process_cmd_hrr(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	uint8_t har_order = 0;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
	if ( ((p2 - p1) > 3) || ((p2 - p1) < 2) ) {
		/* Format ERROR: Send Response */
		rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		/* Get Register Index */
		har_order = *(p1 + 1) - 0x30;
		if ((p2 - p1) == 3) {
			har_order *= 10;
			har_order += *(p1 + 2) - 0x30;
		}
		/* Check Register Index */
		if ((har_order > 0) && (har_order <= DSP_HAR_MAX_ORDER)) {
			/* Set Serial interface to process HAR callback */
			com_har_ptr = com_ptr;
			/* Trigger Harmonic Analysis */
			MetrologySetHarmonicOrder(har_order);
			/* Wait to HAR callback to send serial response */
			rsp_len = 0;
		} else {
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
		}
	}

	if (rsp_len > 0) {
		com_ptr->send_len = rsp_len;

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}
}

static void _process_cmd_idr(com_port_data_t *com_ptr)
{
	/* Send Response */
	com_ptr->send_len = sprintf((char *)com_ptr->send_buff,
				    "Meter ID is : \r\n%c%c%c%c%c%c\r\n",
				    VCom.meterID[0], VCom.meterID[1], VCom.meterID[2],
				    VCom.meterID[3], VCom.meterID[4], VCom.meterID[5]);

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_idw(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		char *p1;
		char *p2;
		uint8_t new_id[METER_ID_SIZE] = {0};

		/* Check Fromat Paramereters */
  		p1 = strstr((const char *)puc_rcv_data, (const char *)"(");
		p2 = strstr((const char *)puc_rcv_data, (const char *)")");
		if ((p2 - p1) != (METER_ID_SIZE + 1)) {
			/* Format ERROR: Send Response */
			puc_send_data = (char *)com_ptr->send_buff;
			com_ptr->send_len = sprintf(puc_send_data, "%s", "Format error. Please use IDW[PSW](xxxxxx)\r\n");
		} else {
			uint8_t i;

			p1++;
			/* Get new ID Data */
			for (i = 0; i < METER_ID_SIZE; i++) {
				new_id[i] = *p1++;
			}

			/* Update Meter ID data */
			memcpy(VCom.meterID, new_id, METER_ID_SIZE);

			/* Write Meter ID to External memory */
			ExtMemWrite(MEM_REG_COMMS_ID, &VCom);

			/* Send Response */
			puc_send_data = (char *)com_ptr->send_buff;
			com_ptr->send_len = sprintf(puc_send_data, "%s", "Set Meter ID is Ok !\r\n");
		}
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_mdc(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];
	puc_send_data = (char *)com_ptr->send_buff;

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		/* Clear all demmand data */
		DemandClear();
		/* Send Response */
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Clear MaxDemand is ok!\r\n");
	} else {
		/* Send Response */
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_mdr(com_port_data_t *com_ptr)
{
	uint8_t rsp_len;
	uint8_t *puc_rcv_data;
	char *puc_send_data;
	char *p1;
	char *p2;
	demand_max_t *prt_demand;
	uint8_t month_idx, month_offset, buf_idx;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];
	/* Check Paramereters: Month */
  	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");

	if ((p1 == NULL) || (p2 == NULL)) {
		/* No parameter. Send all stored months */
		for (month_offset=0; month_offset<12; month_offset++) {
			/* Get data from stored values */
			month_idx = (VRTC.date.month - 1 - month_offset);
			while (month_idx > 12) {
				/* Overflow in month index */
				month_idx += 12;
			}
			prt_demand = &VDemand.max_month[month_idx][0];

			/* Build Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Month MaxDemand is: \r\n", month_offset);
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "TT=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[TOUALL].max/1000,
					   (month_idx+1), prt_demand[TOUALL].time.day, prt_demand[TOUALL].time.hour, prt_demand[TOUALL].time.min);
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T1=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[0].max/1000,
					   (month_idx+1), prt_demand[0].time.day, prt_demand[0].time.hour, prt_demand[0].time.min);
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T2=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[1].max/1000,
					   (month_idx+1), prt_demand[1].time.day, prt_demand[1].time.hour, prt_demand[1].time.min);
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T3=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[2].max/1000,
					   (month_idx+1), prt_demand[2].time.day, prt_demand[2].time.hour, prt_demand[2].time.min);
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T4=%.3fkW %d-%d %02d:%02d\r\n", (float)prt_demand[3].max/1000,
					   (month_idx+1), prt_demand[3].time.day, prt_demand[3].time.hour, prt_demand[3].time.min);

			com_ptr->send_len = rsp_len;

			/* Send response to serial com */
			_send_data(COMPROC_CONSOLE_ID);
		}
	} else if (((p2 - p1) < 2) || ((p2 - p1) > 3)) {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Unsupported Command !\r\n");
		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	} else {
		month_offset = *(p1 + 1) - 0x30;
		if ((p2 - p1) > 2) {
			/* Month offset Parameter is in 2 digits */
			month_offset *= 10;
			month_offset += *(p1 + 2) - 0x30;
		}

		/* Get data from stored values */
		month_idx = (VRTC.date.month - 1 - month_offset);
		while (month_idx > 12) {
			/* Overflow in month index */
			month_idx += 12;
		}
		prt_demand = &VDemand.max_month[month_idx][0];

		/* Build Response */
		rsp_len = sprintf((char *)com_ptr->send_buff, "Last %d Month MaxDemand is: \r\n", month_offset);
		rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "TT=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[TOUALL].max/1000,
				   (month_idx+1), prt_demand[TOUALL].time.day, prt_demand[TOUALL].time.hour, prt_demand[TOUALL].time.min);
		rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T1=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[0].max/1000,
				   (month_idx+1), prt_demand[0].time.day, prt_demand[0].time.hour, prt_demand[0].time.min);
		rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T2=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[1].max/1000,
				   (month_idx+1), prt_demand[1].time.day, prt_demand[1].time.hour, prt_demand[1].time.min);
		rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T3=%.3fkW %d-%d %02d:%02d ", (float)prt_demand[2].max/1000,
				   (month_idx+1), prt_demand[2].time.day, prt_demand[2].time.hour, prt_demand[2].time.min);
		rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T4=%.3fkW %d-%d %02d:%02d\r\n", (float)prt_demand[3].max/1000,
				   (month_idx+1), prt_demand[3].time.day, prt_demand[3].time.hour, prt_demand[3].time.min);

		com_ptr->send_len = rsp_len;

		/* Send response to serial com */
		_send_data(COMPROC_CONSOLE_ID);
	}
}

static void _process_cmd_par(com_port_data_t *com_ptr)
{
	char *p1;
	char *p2;
	uint8_t rsp_len;
	char signT = ' ';
	char signA = ' ';
	char signB = ' ';
	char signC = ' ';
	char signN = ' ';
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][3];

	/* Check Fromat Paramereters */
	p1 = strstr((const char *)puc_rcv_data, (const char *)"[");
	p2 = strstr((const char *)puc_rcv_data, (const char *)"]");
	if ((p2 - p1) != 2) {
		/* Format ERROR: Send Response */
		rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
	} else {
		/* Get Measurement Type */
		switch (*(p1 + 1)) {
		case 'U':
			/* Read Voltage */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Present voltage is : \r\n");
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "Ua=%.3fV Ub=%.3fV Uc=%.3fV\r\n",
					   (float)VAFE.RMS[Ua]/10000, (float)VAFE.RMS[Ub]/10000, (float)VAFE.RMS[Uc]/10000);
			break;

		case 'I':
			/* Read Current */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Present current is : \r\n");
                        rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "Ia=%.4fA Ib=%.4fA Ic=%.4fA Ini=%.4fA Inm=%.4fA Inmi=%.4fA\r\n",
					   (float)VAFE.RMS[Ia]/10000, (float)VAFE.RMS[Ib]/10000, (float)VAFE.RMS[Ic]/10000,
                                           (float)VAFE.RMS[Ini]/10000, (float)VAFE.RMS[Inm]/10000, (float)VAFE.RMS[Inmi]/10000);
			break;

		case 'Q':
			/* Get signs of the measurements */
			if (VAFE.ST.BIT.qt_dir) {
				signT = '-';
			}
			if (VAFE.ST.BIT.qa_dir) {
				signA = '-';
			}
			if (VAFE.ST.BIT.qb_dir) {
				signB = '-';
			}
			if (VAFE.ST.BIT.qc_dir) {
				signC = '-';
			}
			/* Read Reactive Power */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Present reactive power is : \r\n");
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "Qt=%c%.1fVar Qa=%c%.1fVar Qb=%c%.1fVar Qc=%c%.1fVar\r\n",
					   signT, (float)VAFE.RMS[Qt]/10, signA, (float)VAFE.RMS[Qa]/10,
					   signB, (float)VAFE.RMS[Qb]/10, signC, (float)VAFE.RMS[Qc]/10);
			break;

		case 'P':
			/* Get signs of the measurements */
			if (VAFE.ST.BIT.pt_dir) {
				signT = '-';
			}
			if (VAFE.ST.BIT.pa_dir) {
				signA = '-';
			}
			if (VAFE.ST.BIT.pb_dir) {
				signB = '-';
			}
			if (VAFE.ST.BIT.pc_dir) {
				signC = '-';
			}
			/* Read Active Power */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Present active power is : \r\n");
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "Pt=%c%.1fW Pa=%c%.1fW Pb=%c%.1fW Pc=%c%.1fW\r\n",
					   signT, (float)VAFE.RMS[Pt]/10, signA, (float)VAFE.RMS[Pa]/10,
					   signB, (float)VAFE.RMS[Pb]/10, signC, (float)VAFE.RMS[Pc]/10);
			break;

		case 'S':
			/* Read Aparent Power */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Present apparent power is : \r\n");
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "St=%.1fVA Sa=%.1fVA Sb=%.1fVA Sc=%.1fVA\r\n",
					   (float)VAFE.RMS[St]/10, (float)VAFE.RMS[Sa]/10, (float)VAFE.RMS[Sb]/10, (float)VAFE.RMS[Sc]/10);
			break;

		case 'F':
			/* Read Frecuency */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Present frequency is : \r\nFreq=%.2fHz\r\n", (float)VAFE.RMS[Freq]/100);
			break;

		case 'A':
			/* Get signs of the measurements */
			if (VAFE.RMS[AngleA] & 0x80000000) {
				signA = '-';
			}
			if (VAFE.RMS[AngleB] & 0x80000000) {
				signB = '-';
			}
			if (VAFE.RMS[AngleC] & 0x80000000) {
				signC = '-';
			}
			if (VAFE.RMS[AngleN] & 0x80000000) {
				signN = '-';
			}
			/* Read Angle */
			rsp_len = sprintf((char *)com_ptr->send_buff, "Voltage and current angle is : \r\n");
			#if BOARD==PIC32CXMTC_DB
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "Angle_A=%c%.3f Angle_B=%c%.3f Angle_C=%c%.3f Angle_N=%c%.3f\r\n",
					   signA, (float)(VAFE.RMS[AngleA] & 0xFFFFF)/1000,
					   signB, (float)(VAFE.RMS[AngleB] & 0xFFFFF)/1000,
					   signC, (float)(VAFE.RMS[AngleC] & 0xFFFFF)/1000,
					   signN, (float)(VAFE.RMS[AngleN] & 0xFFFFF)/1000);
			#else
			rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "Angle_A=%c%.3f Angle_B=%c%.3f\r\n",
					   signA, (float)(VAFE.RMS[AngleA] & 0xFFFFF)/1000,
					   signB, (float)(VAFE.RMS[AngleB] & 0xFFFFF)/1000);
			#endif


			break;

		default:
			/* Format ERROR: Send Response */
			rsp_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
		}
	}

	com_ptr->send_len = rsp_len;

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_rtcr(com_port_data_t *com_ptr)
{
	rtc_t rtc_data;

	rtc_data = VRTC;
	/* Send Response */
	com_ptr->send_len = sprintf((char *)com_ptr->send_buff,
				    "Present RTC is (yy-mm-dd w hh:mm:ss): \r\n%02u-%02u-%02u %u %02u:%02u:%02u\r\n",
				    rtc_data.date.year - 2000, rtc_data.date.month, rtc_data.date.day, rtc_data.date.week,
				    rtc_data.time.hour, rtc_data.time.minute, rtc_data.time.second);

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_rtcw(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][4];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		char *p1;
		char *p2;

		/* Check Format Paramereters */
  		p1 = strstr((const char *)puc_rcv_data, (const char *)"(");
		p2 = strstr((const char *)puc_rcv_data, (const char *)")");
		if ((p2 - p1) != 21) {
			/* Format ERROR: Send Response */
			puc_send_data = (char *)com_ptr->send_buff;
			com_ptr->send_len = sprintf(puc_send_data, "%s", "Unsupported Command !\r\n");
		} else {
			rtc_t rtc_data;
			uint8_t num1, num2;
			uint8_t format_err = 0;

			/* Get RTC data */
			p1++;
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.date.year = 2000 + CHARTODEC(num1) * 10 + CHARTODEC(num2);
			/* Check format '-' */
			if (*p1++ != '-') {
				format_err = 1;
			}
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.date.month = CHARTODEC(num1) * 10 + CHARTODEC(num2);
			/* Check format '-' */
			if (*p1++ != '-') {
				format_err = 1;
			}
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.date.day = CHARTODEC(num1) * 10 + CHARTODEC(num2);
			/* Check format ' ' */
			if (*p1++ != ' ') {
				format_err = 1;
			}
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.date.week = CHARTODEC(num2);
			/* Check format ' ' */
			if (*p1++ != ' ') {
				format_err = 1;
			}
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.time.hour = CHARTODEC(num1) * 10 + CHARTODEC(num2);
			/* Check format '-' */
			if (*p1++ != ':') {
				format_err = 1;
			}
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.time.minute = CHARTODEC(num1) * 10 + CHARTODEC(num2);
			/* Check format '-' */
			if (*p1++ != ':') {
				format_err = 1;
			}
			num1 = *p1++;
			num2 = *p1++;
			rtc_data.time.second = CHARTODEC(num1) * 10 + CHARTODEC(num2);

			/* Update RTC data */
			if (format_err || RTCProcSetNewRTC(&rtc_data)) {
				/* Format ERROR: Send Response */
				puc_send_data = (char *)com_ptr->send_buff;
				com_ptr->send_len = sprintf(puc_send_data, "%s", "Unsupported Command !\r\n");
			} else {
				/* Send Response */
				puc_send_data = (char *)com_ptr->send_buff;
				com_ptr->send_len = sprintf(puc_send_data, "%s", "Set RTC is Ok !\r\n");

				// Store VRTC data in external memory
				ExtMemWrite(MEM_REG_RTC_ID, &VRTC);
			}
		}
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_tour(com_port_data_t *com_ptr)
{
	uint8_t tou_idx;
	uint8_t rsp_len;

	/* Send Response */
	rsp_len = sprintf((char *)com_ptr->send_buff, "TOU table is :\r\n");
	for (tou_idx = 0; tou_idx < VTou.num; tou_idx++) {
		rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "T%d=%02d:%02d %d ",
				   (tou_idx+1), VTou.time_slot[tou_idx].hour, VTou.time_slot[tou_idx].minute, (VTou.time_slot[tou_idx].rate_id+1));
	}
	rsp_len += sprintf((char *)&com_ptr->send_buff[rsp_len], "\r\n");

	com_ptr->send_len = rsp_len;

	/* Send response to serial com */
	_send_data(COMPROC_CONSOLE_ID);
}

static void _process_cmd_touw(com_port_data_t *com_ptr)
{
	uint8_t buf_idx;
	uint8_t *puc_rcv_data;
	char *puc_send_data;

	buf_idx = com_ptr->bufidx ^ 0x01;
	puc_rcv_data = &com_ptr->rcv_buff[buf_idx][4];

	/* Check Secure Password */
	if (_check_sec_pwd(puc_rcv_data)) {
		char *p1;
		char *p2;
		tou_t new_vtou = {0};

		/* Check Fromat Paramereters */
  		p1 = strstr((const char *)puc_rcv_data, (const char *)"(");
		p2 = strstr((const char *)puc_rcv_data, (const char *)")");
		new_vtou.time_slot_idx = 0;
		new_vtou.num = (p2 - p1) / 9;
		if ((p2 - p1) % 9) {
			/* Format ERROR: Send Response */
			puc_send_data = (char *)com_ptr->send_buff;
			com_ptr->send_len = sprintf(puc_send_data, "%s", "Unsupported Command !\r\n");
		} else {
			uint8_t num1, num2, i;

			/* Get new TOU Data */
			for (i = 0; i < new_vtou.num; i++) {
				p1++;
				num1 = *p1++;
				num2 = *p1++;
				new_vtou.time_slot[i].hour = CHARTODEC(num1) * 10 + CHARTODEC(num2);
				/* Skip ':' */
				p1++;
				num1 = *p1++;
				num2 = *p1++;
				new_vtou.time_slot[i].minute = CHARTODEC(num1) * 10 + CHARTODEC(num2);
				/* Skip space and zero data */
				p1 += 2;
				num1 = *p1++;
				new_vtou.time_slot[i].rate_id = (tou_rate_id_t)CHARTODEC(num1-1);
			}

			/* Update Global VTou data */
			VTou = new_vtou;

                        /* Call TOUPrrocess, to detect changes of slot without waiting for a new minute to start */
                        TOUProcess();

			/* Save in memory */
			ExtMemWrite(MEM_REG_TOU_ID, &VTou);

			/* Send Response */
			puc_send_data = (char *)com_ptr->send_buff;
			com_ptr->send_len = sprintf(puc_send_data, "%s", "Set TOU is Ok !\r\n");
		}
	} else {
		/* Send Response */
		puc_send_data = (char *)com_ptr->send_buff;
		com_ptr->send_len = sprintf(puc_send_data, "%s", "Password Error !\r\n");
	}

	_send_data(COMPROC_CONSOLE_ID);
}

static terminal_cmd_t _get_terminal_cmd(uint8_t *puc_data)
{
	terminal_cmd_t cmd = TER_CMD_INVALID;

	if ((*puc_data == 'B') && (*(puc_data + 1) == 'U') && (*(puc_data + 2) == 'F')) {
		cmd = TER_CMD_BUF;
	} else if((*puc_data == 'C') && (*(puc_data + 1) == 'A') && (*(puc_data + 2) == 'L')) {
		cmd = TER_CMD_CAL;
	} else if((*puc_data == 'C') && (*(puc_data + 1) == 'N') && (*(puc_data + 2) == 'F')) {
		cmd = TER_CMD_CNF;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'A') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_DAR;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'C') && (*(puc_data + 2) == 'B')) {
		cmd = TER_CMD_DCB;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'C') && (*(puc_data + 2) == 'D')) {
		cmd = TER_CMD_DCD;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'C') && (*(puc_data + 2) == 'M')) {
		cmd = TER_CMD_DCM;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'C') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_DCR;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'C') && (*(puc_data + 2) == 'S')) {
		cmd = TER_CMD_DCS;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'C') && (*(puc_data + 2) == 'W')) {
		cmd = TER_CMD_DCW;
	} else if((*puc_data == 'D') && (*(puc_data + 1) == 'S') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_DSR;
	} else if((*puc_data == 'E') && (*(puc_data + 1) == 'N') && (*(puc_data + 2) == 'C')) {
		cmd = TER_CMD_ENC;
	} else if((*puc_data == 'E') && (*(puc_data + 1) == 'N') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_ENR;
	} else if((*puc_data == 'E') && (*(puc_data + 1) == 'V') && (*(puc_data + 2) == 'E') && (*(puc_data + 3) == 'C')) {
		cmd = TER_CMD_EVEC;
	} else if((*puc_data == 'E') && (*(puc_data + 1) == 'V') && (*(puc_data + 2) == 'E') && (*(puc_data + 3) == 'R')) {
		cmd = TER_CMD_EVER;
	} else if((*puc_data == 'H') && (*(puc_data + 1) == 'A') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_HAR;
	} else if((*puc_data == 'H') && (*(puc_data + 1) == 'R') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_HRR;
	} else if((*puc_data == 'I') && (*(puc_data + 1) == 'D') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_IDR;
	} else if((*puc_data == 'I') && (*(puc_data + 1) == 'D') && (*(puc_data + 2) == 'W')) {
		cmd = TER_CMD_IDW;
	} else if((*puc_data == 'M') && (*(puc_data + 1) == 'D') && (*(puc_data + 2) == 'C')) {
		cmd = TER_CMD_MDC;
	} else if((*puc_data == 'M') && (*(puc_data + 1) == 'D') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_MDR;
	} else if((*puc_data == 'P') && (*(puc_data + 1) == 'A') && (*(puc_data + 2) == 'R')) {
		cmd = TER_CMD_PAR;
	} else if((*puc_data == 'R') && (*(puc_data + 1) == 'T') && (*(puc_data + 2) == 'C') && (*(puc_data + 3) == 'R')) {
		cmd = TER_CMD_RTCR;
	} else if((*puc_data == 'R') && (*(puc_data + 1) == 'T') && (*(puc_data + 2) == 'C') && (*(puc_data + 3) == 'W')) {
		cmd = TER_CMD_RTCW;
	} else if((*puc_data == 'T') && (*(puc_data + 1) == 'O') && (*(puc_data + 2) == 'U') && (*(puc_data + 3) == 'R')) {
		cmd = TER_CMD_TOUR;
	} else if((*puc_data == 'T') && (*(puc_data + 1) == 'O') && (*(puc_data + 2) == 'U') && (*(puc_data + 3) == 'W')) {
		cmd = TER_CMD_TOUW;
	} else if((*puc_data == 'R') && (*(puc_data + 1) == 'S') && (*(puc_data + 2) == 'T')) {
		cmd = TER_CMD_RST;
	} else if((*puc_data == 'R') && (*(puc_data + 1) == 'L') && (*(puc_data + 2) == 'D')) {
		cmd = TER_CMD_RLD;
	}

	return cmd;
}

/**
 * \brief com receive byte process
 *
 * \param proc_id Comm Port Identifier.
 *
 * \return 1 when complete command should be processed, 0 otherwise
 */
static uint8_t _process_rcv_data(com_port_id_t proc_id)
{
	com_port_data_t *com_ptr;
	uint8_t buf_idx;
	uint8_t uc_res = 0;

	VCom.lamptimer = 2;

	com_ptr = &VCom.comport[proc_id];
	buf_idx = com_ptr->bufidx;

	if (usart_read(com_ptr->p_usart, &com_ptr->rcv_data) == 0) {
		/* Check Rcv State */
		if (com_ptr->state[buf_idx] >= PROCESS) {
			/* No free space, lost byte */
			return 0;
		}

		com_ptr->overtimer = 0;
		if (com_ptr->protocolmode == ASCII_PROTOCOL) {
			/* Check Backspace */
			if (com_ptr->rcv_data == '\b' || com_ptr->rcv_data == 0x7F) {
				if (com_ptr->offset[buf_idx] > 0) {
					if (VCom.console_echo_on) {
						if (com_ptr->state[buf_idx^0x01] <= RECDATA) {
							usart_write_line(com_ptr->p_usart, del_cmd);
						} else {
							/* Set Echo Pending flag */
							com_ptr->echo_pending[buf_idx] = 1;
						}
					}
					/* Overwrite previous byte */
					com_ptr->offset[buf_idx]--;
				}
			} else if ((com_ptr->rcv_data == 0x0A) && (com_ptr->offset[buf_idx] > 0)) {
				/* Overwrite previous byte */
				com_ptr->offset[buf_idx]--;
			} else {
				/* Check Uppercase */
				if (ISCHAR(com_ptr->rcv_data)) {
					com_ptr->rcv_data = UPPER(com_ptr->rcv_data);
				}

				/* ECHO */
				if (VCom.console_echo_on) {
					/* Check data processing from ping pong buffer */
					if (com_ptr->state[buf_idx^0x01] <= RECDATA) {
						usart_write(com_ptr->p_usart, com_ptr->rcv_data);
					} else {
						/* Set Echo Pending flag */
						com_ptr->echo_pending[buf_idx] = 1;
					}
				}

				/* Check FREE State */
				if (com_ptr->state[buf_idx] == FREE) {
					com_ptr->offset[buf_idx] = 0;
					com_ptr->state[buf_idx] = RECDATA;
				}

				/* Add received data to internal buffer */
				com_ptr->rcv_buff[buf_idx][com_ptr->offset[buf_idx]] = com_ptr->rcv_data;
				com_ptr->offset[buf_idx]++;

				/* Check Overflow */
				if (com_ptr->offset[buf_idx] >= FRAME_MAX_RCV_LEN) {
					/* Discard message */
					com_ptr->state[buf_idx] = FREE;
					/* restart offset for the next buffer */
					com_ptr->offset[buf_idx] = 0;
				} else if (com_ptr->rcv_data == 0x0D) {
					/* Check CR */
					com_ptr->state[buf_idx] = PROCESS;
					/* Swap Receive Buffer (ping pong buffer) */
					com_ptr->bufidx ^= 0x01;
					/* Command is completed */
					uc_res = 1;
				}
			}
		} else {
			/* Add received data to internal buffer */
			com_ptr->rcv_buff[buf_idx][com_ptr->offset[buf_idx]] = com_ptr->rcv_data;
			com_ptr->offset[buf_idx]++;

			/* Check Overflow */
			if (com_ptr->offset[buf_idx] >= FRAME_MAX_RCV_LEN) {
				/* Discard message */
				com_ptr->state[buf_idx] = FREE;
				/* restart offset for the next buffer */
				com_ptr->offset[buf_idx] = 0;
			} else {
				com_ptr->state[buf_idx] = PROCESS;
				/* Swap Receive Buffer (ping pong buffer) */
				com_ptr->bufidx ^= 0x01;
				/* Command in completed */
				uc_res = 1;
			}
		}
	}

	return uc_res;
}

/**
 * \brief CONSOLE interrupt Handler
 */
uint32_t ul_cnt = 0;
void CONF_COMMAND_UART_Handler(void)
{
	if (_process_rcv_data(COMPROC_CONSOLE_ID)) {
		TaskPutIntoQueue(CommandConsoleProcess);
		ul_cnt++;
	}
}

/**
 * \brief OPTO UART interrupt Handler
 */
void CONF_OPTO_UART_Handler(void)
{
	if (_process_rcv_data(COMPROC_OPTO_ID)) {
		TaskPutIntoQueue(CommandOptoProcess);
	}
}

/**
 * \brief Comprocess Initialization.
 *
 * \note HW serial ports must be initialized in main application
 */
void CommandInit(void)
{
	command_t com_temp;

	/* Init comprocess data */
	memset(&VCom, 0, sizeof(command_t));

	/* Get METER ID for External Memory (user defined value) */
	ExtMemRead(MEM_REG_COMMS_ID, &com_temp);
	if ( com_temp.meterID[0] == 0xFF && com_temp.meterID[1] == 0xFF && com_temp.meterID[2] == 0xFF && com_temp.meterID[3] == 0xFF && com_temp.meterID[4] == 0xFF && com_temp.meterID[5] == 0xFF){
		ExtMemGetUniqueId(&VCom.meterID[0], 6);
	} else {
		memcpy(VCom.meterID, com_temp.meterID, METER_ID_SIZE);
	}

	/* Set Callback for Harmonic calculations */
	MetrologySetHarmonicsCallback(_har_callback);

	/* Set Callback for Calibration Process */
	MetrologySetCalibrationCallback(_cal_callback);

	/* Enable ECHO in Console */
	VCom.console_echo_on = COMPROC_ECHO_ON;

	/* Configure ASCII protocol for console port */
	VCom.comport[COMPROC_CONSOLE_ID].protocolmode = ASCII_PROTOCOL;

	/* Init PDC for Console port */
	VCom.comport[COMPROC_CONSOLE_ID].p_usart = CONF_COMMAND_UART;
	VCom.comport[COMPROC_CONSOLE_ID].pdc_base = usart_get_pdc_base(CONF_COMMAND_UART);

	/* Init PDC for Opto port */
	VCom.comport[COMPROC_OPTO_ID].p_usart = (Usart *)CONF_OPTO_UART;
	VCom.comport[COMPROC_OPTO_ID].pdc_base = uart_get_pdc_base(CONF_OPTO_UART);

	/* Enable receiver interrupt */
	usart_enable_interrupt(CONF_COMMAND_UART, US_IER_RXRDY);
	/* Enable interrupt */
	NVIC_EnableIRQ(CONF_COMMAND_UART_IRQn);

	/* Send Console Prompt */
	_send_console_prompt();

	/* Enable receiver interrupt */
	usart_enable_interrupt((Usart *)CONF_OPTO_UART, US_IER_RXRDY);
	/* Enable interrupt */
	NVIC_EnableIRQ(CONF_OPTO_UART_IRQn);
}


/**
 * \brief Opto Protocol Process
 *
 * \param puc_data  Pointer to the data
 * \param uc_len    Length of the data in bytes
 *
 * \note
 */
void CommandOptoProcess(void)
{

}

/**
 * \brief Console Protocol Process
 *
 * \param puc_data  Pointer to the data
 * \param uc_len    Length of the data in bytes
 *
 * \note
 */
void CommandConsoleProcess(void)
{
	com_port_data_t *com_ptr;
	terminal_cmd_t com_cmd;
	uint8_t buf_idx;

	com_ptr = &VCom.comport[COMPROC_CONSOLE_ID];
	buf_idx = com_ptr->bufidx^0x01;

	while (com_ptr->state[buf_idx] == PROCESS) {
		/* Update State */
		com_ptr->state[buf_idx] = SEND;

		if (com_ptr->echo_pending[buf_idx]) {
			/* Clear Echo Pending flag */
			com_ptr->echo_pending[buf_idx] = 0;
			/* Send ECHO command */
			com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s\r\n", (char *)&com_ptr->rcv_buff[buf_idx]);
		} else {
			/* Send CR/LF */
			com_ptr->send_buff[0] = 0x0D;
			com_ptr->send_buff[1] = 0x0A;
			com_ptr->send_len = 2;
		}
		_send_data(COMPROC_CONSOLE_ID);

		com_cmd = _get_terminal_cmd(com_ptr->rcv_buff[buf_idx]);

		switch(com_cmd) {
		case TER_CMD_BUF:
			_process_cmd_buf(com_ptr);
			break;

		case TER_CMD_CAL:
			_process_cmd_cal(com_ptr);
			break;

		case TER_CMD_CNF:
			_process_cmd_cnf(com_ptr);
			break;

		case TER_CMD_DAR:
			_process_cmd_dar(com_ptr);
			break;

		case TER_CMD_DCB:
			_process_cmd_dcb(com_ptr);
			break;

		case TER_CMD_DCD:
			_process_cmd_dcd(com_ptr);
			break;

		case TER_CMD_DCM:
			_process_cmd_dcm(com_ptr);
			break;

		case TER_CMD_DCR:
			_process_cmd_dcr(com_ptr);
			break;

		case TER_CMD_DCS:
			_process_cmd_dcs(com_ptr);
			break;

		case TER_CMD_DCW:
			_process_cmd_dcw(com_ptr);
			break;

		case TER_CMD_DSR:
			_process_cmd_dsr(com_ptr);
			break;

		case TER_CMD_ENC:
			_process_cmd_enc(com_ptr);
			break;

		case TER_CMD_ENR:
			_process_cmd_enr(com_ptr);
			break;

		case TER_CMD_EVEC:
			_process_cmd_evec(com_ptr);
			break;

		case TER_CMD_EVER:
			_process_cmd_ever(com_ptr);
			break;

		case TER_CMD_HAR:
			_process_cmd_har(com_ptr);
			break;

		case TER_CMD_HRR:
			_process_cmd_hrr(com_ptr);
			break;

		case TER_CMD_IDR:
			_process_cmd_idr(com_ptr);
			break;

		case TER_CMD_IDW:
			_process_cmd_idw(com_ptr);
			break;

		case TER_CMD_MDC:
			_process_cmd_mdc(com_ptr);
			break;

		case TER_CMD_MDR:
			_process_cmd_mdr(com_ptr);
			break;

		case TER_CMD_PAR:
			_process_cmd_par(com_ptr);
			break;

		case TER_CMD_RTCR:
			_process_cmd_rtcr(com_ptr);
			break;

		case TER_CMD_RTCW:
			_process_cmd_rtcw(com_ptr);
			break;

		case TER_CMD_TOUR:
			_process_cmd_tour(com_ptr);
			break;

		case TER_CMD_TOUW:
			_process_cmd_touw(com_ptr);
			break;

		case TER_CMD_RST:
			_process_cmd_rst(com_ptr);
			break;

		case TER_CMD_RLD:
			_process_cmd_rld(com_ptr);
			break;

		default:
			/* Unknown terminal command */
			com_ptr->send_len = sprintf((char *)com_ptr->send_buff, "%s", "Unsupported Command !\r\n");
			_send_data(COMPROC_CONSOLE_ID);
			break;
		}

		/* Update State */
		com_ptr->state[buf_idx] = FREE;
		/* Restart Rcv buffer */
		memset(&com_ptr->rcv_buff[buf_idx], 0, FRAME_MAX_RCV_LEN);

		/* Check the other buffer */
		buf_idx ^= 0x01;
	}

	/* Send Console Prompt */
	_send_console_prompt();
}

/**
 * \brief Send message through console port
 *
 * \param msg    Pointer to the message
 */
void CommandSendConsoleMsg(const char *msg)
{
	if (msg != NULL) {
		VCom.comport[COMPROC_CONSOLE_ID].send_len = sprintf((char *)VCom.comport[COMPROC_CONSOLE_ID].send_buff, "%s", msg);
		_send_data(COMPROC_CONSOLE_ID);
	}
}

///**
/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

