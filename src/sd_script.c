/** \file sd_script.c
 * \brief \b "SCRIPT PLAYER"
 * \n M����� ��������������� �������� � SD-�����
 *
 * ������ ��������, ��� ������ ����. ������ ��������� ������ ��������� �������,
 * ������� ����� ���� �������� ������ �������������.
 * \author ARV
 * \date	15 ����. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * ��� ���������� ���������:\n
 * 	-# AVR GCC 5.2.1 ��� ����� ����� ������
 *
 * \addtogroup BGEFFECTS
 * \copybrief sd_script.c ��. \ref sd_script.c
 */

#include <avr/io.h>
#include <avr_helper.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "global.h"
#include "pixel.h"
#include "color_transform.h"
#include "avr_helper.h"
#include "hardware.h"
#include "main_effect.h"
#include "lcd_show.h"
#include <util/delay.h>
#include "PFFS/pff.h"
#include "sd_script.h"
#if !defined(DISABLE_SD_SCRIPT)
/**
 * @{
 */

// ������� - ������ WS2812b
#define enter_sd_mode()		do { SPCR = _BV(SPE) | _BV(MSTR); PORT(WS_LOCK_PORT) &= ~WS_LOCK_PIN;} while(0)
// ������� - ���������� WS2812b
#define leave_sd_mode()		do { SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA); SPDR=0;  while(!(SPSR & (1<<SPIF))); PORT(WS_LOCK_PORT) |= WS_LOCK_PIN; } while(0)

#define MODULE_NAME		"SCRIPT PLAYER"

static __flash const char _name[] = MODULE_NAME;

/// ��������� �������
typedef struct{
	char		filename[13];	///< ��� ����� �������
	uint32_t	pos;			///< ��������� ������� � ����� ��� ������
	uint16_t 	readed;			///< ��������� � ��������� ��� ���-�� ������
	char 		*cmd;
} script_t;

/// ������� ������
static script_t	script;

/// ��������� - ������ ����� ������ � �����. ����� ���� ������, �� �� ������ 256
#define SD_BUF_SZ	256UL

/// ��� �������� ��������� ������
typedef enum{
	SCRIPT_OK,			///< ���������� ������
	SCRIPT_NO_SD,		///< ����� �� ���������
	SCRIPT_NOTFOUND,	///< ���� ������� �� ������ �� �����
	SCRIPT_ERROR_IO,	///< ������ ������ � �����
	SCRIPT_INFLOOP		///< � ������� ���� ����������� ����
#if defined(_DEBUG_)
	,DBG_ERROR			// ���������� ���������
#endif
} script_result_t;

static script_result_t	sdcard_open(void);
static uint16_t get_script_count(void);
static script_result_t	script_open(script_t *s, uint16_t id);

/// ���������� ������ �� ����� � �����
static script_result_t script_read_str(script_t *s, char *str);
static script_result_t script_seek(script_t *s, uint32_t pos);
static void script_exec(signal_t *s);

static int8_t select_next = 0;

/// ��������� - ���������� ���������� � �������, �� ������
#define VAR_CNT		16
/// ������� ����������� ������ � �������
#define STACK_DEPTH	5

/// ��������� ���������� �������
typedef struct{
	uint8_t		bright;			///< �������
	uint8_t		fade;			///< ���������
	rgb_t		color;			///< ����
	uint8_t		var[VAR_CNT];	///< ����������
	uint8_t		rev;			///< ������
	uint8_t		stack_head;		///< "������" ����� ������
	uint32_t	loop_stack[STACK_DEPTH];///< ���� ������
} script_param_t;

/// ��������� �������
static script_param_t	param;
/// ��������� �� ����� �����������
static char *buf;
/// ������� ���������
static script_result_t status;
/// ���������� ������ �� ��������� �� �����
static uint16_t total_script_files;
/// ����� �������� ������������ �������
static uint16_t script_id;
/// �������� �������
static FATFS	fs;
/// ������� ����������
static DIR		dir;

/** ����������� SD-�����
 *
 * @return ��������� �����������
 */
static script_result_t	sdcard_open(void){
	// ��������� �������� �������
	if(pf_mount(&fs) == FR_OK){
		// ���� �������������� - ��������� �������� ����������
		if(pf_opendir(&dir, "") == FR_OK){
			// ���� ��������� - ��
			return SCRIPT_OK;
		}
	}
	// ��� ����� ���� ���������� ������
	return SCRIPT_NO_SD;
}

/**
 *
 * @return ���-�� ��������� ������ ��������
 */
static uint16_t get_script_count(void){
	FRESULT res;
	FILINFO file;
	uint16_t count = 0;

	// ������������� ������������ ����������
	res = pf_readdir(&dir, 0);
	// ���� ������������ �������
	while(res == FR_OK){
		// ��������� ��������� ������ � �����
		res = pf_readdir(&dir, &file);
		// ���������� ���� ��� ������ ��� ����� �������
		if (res || !file.fname[0]) break;
		// ���������, ��� ������ - �� ���������� � �� ������� ����,
		// � ��� ���� ���������� � ���� ����������
		if (!(file.fattrib & (AM_DIR|AM_HID)) && strstr(file.fname, ".SC"))
			count++; // ����� ������� "����������" ������
	}
	return count;
}

/**
 *
 * @param s ������ �� ������ �������
 * @param id ����� �����
 * @return ��������� ��������
 */
static script_result_t	script_open(script_t *s, uint16_t id){
	FRESULT res;
	FILINFO file;
	// ����� ������������� � 0, �� �� ���������� - � 1, ������� ������������ �����
	id++;
	// ����������� ������ ���������� ������ � ����������
	res = pf_readdir(&dir, 0);
	do {
		res = pf_readdir(&dir, &file);
		if (res || !file.fname[0]) break;
		if (!(file.fattrib & (AM_DIR|AM_HID)) && strstr(file.fname, ".SC")) id--;
	} while(id && (res == FR_OK));

	if(res != FR_OK)
		return SCRIPT_NO_SD;
	else {
		// ���� ��������� �� ���������� ������ ����� - �������������� ������ �������
		memset(s, 0, sizeof(script_t));
		memset(&param, 0, sizeof(script_param_t));
		memset(buf, 0, SD_BUF_SZ);
		strncpy(s->filename, file.fname, 12);
		// � ������� ������� ���
		return pf_open(s->filename) == FR_OK ? SCRIPT_OK : SCRIPT_NOTFOUND;
	}
}

/**
 * ���� ��� ���������� �������� ������, ����� � ������ ������� ����� ��������� 0
 * @param s ��������� �� ������
 * @param str ��������� �� �����
 * @return ��������� ��������
 */
static script_result_t script_read_str(script_t *s, char *str){
	uint16_t n = SD_BUF_SZ;
	if(status == SCRIPT_OK){
		// ������������� ����
		if(pf_lseek(s->pos) == FR_OK){
			// ������ � ���
			if(pf_read(str, SD_BUF_SZ, &s->readed) == FR_OK){
				// ������������ ��������� ���-�� ������
				if(s->readed < n) n = s->readed;
				// ���� ��������� "�����" ������� � ����� �� ������ ������� ����� ������
				uint8_t b = n-1;
				// ����� "�����" ������� �����: ���� ������ ������-����������� �� ����� ��������� ������
				while(n && str[b] && (str[b] != ' ') && (str[b] != '\n') && (str[b] != '\r') && (str[b] != '\t')) {
					n--;
					b = n-1;
				}
				// �������� ��������� �����
				str[n] = 0;
				// ������������ ��������� ���������� � ������� �����
				s->readed = n;
				s->pos += n;
				return SCRIPT_OK;
			}
		}
	}
	return SCRIPT_ERROR_IO;
}

/**
 *
 * @param s ��������� �� ������ �������
 * @param pos ����� ������� � �����
 * @return ��������� ��������
 */
static script_result_t script_seek(script_t *s, uint32_t pos){
	s->pos = pos;
	return pf_lseek(pos) == FR_OK ? SCRIPT_OK : SCRIPT_ERROR_IO;
}

/**
 * ���������� ����������������� �����
 * @param str ��������� �� ������ � ������
 * @return �������� 0...15 ��� 255, ���� ������ ���������
 */
static uint8_t get_script_digit(char *str){
	if((str[0] >= '0') && (str[0] <= '9')) return str[0] - '0';
	else if((str[0] >= 'A') && (str[0] <= 'F')) return str[0] - 'A' + 10;
	else return 255;
}

#define ERROR_CH	0xFF00

/**
 * ���������� ����� ������� �� ������
 * \note ��� ������ ���������� � ��� ����� � �������� ���������� ��� �������.
 * @param str ��������� �� �����
 * @return �������� ����� � �������� 0...255 ��� 0xFF00 � ������ ������������� ����� � ������
 */
static uint16_t get_script_val(char *str){
	// ��� ����������?
	if(str[0] == 'V'){
		// ��������� ����� ����������
		uint8_t n = get_script_digit(++str);
		// ����� ���������� ������ ���� ����������
		return n < VAR_CNT ? param.var[n] : ERROR_CH;
	} else if (str[0] == 'R'){
		// ������� RD ��� RP
		if(str[1] == 'D') return rand() & 0x00FF;
		else if(str[1] == 'P') return rand() % PIXEL_CNT;
		else return ERROR_CH;
	} else if (str[0] == 'T'){
		// ������� TP
		if(str[1] == 'P') return PIXEL_CNT-1;
		else return ERROR_CH;
	} else {
		// ����
		uint8_t nh = get_script_digit(str++);
		uint8_t nl = get_script_digit(str);
		if((nh < 0x10) && (nl < 0x10)) return (nh << 4) | nl;
		else return ERROR_CH;
	}
}

/**
 * ��������� ������� ����� �� ����� ������
 * @return ����� �������
 */
static uint32_t popstack(void){
	// ���� ������ ���� ��������
	if(param.stack_head != 0){
		// �������� "������"
		param.stack_head--;
		// ���������� �������
		return param.loop_stack[param.stack_head];
	}
	else // ���� ���� ���� - �������� � ������ ����� ���������
		return 0;
}

/**
 * ���������� � ���� ������
 * @param d ������� ������ ������� �����
 */
static void pushstack(uint32_t d){
	// � ����� ������ ���� �����
	if(param.stack_head < (STACK_DEPTH-1)){
		param.loop_stack[param.stack_head++] = d;
	}
	// � ���� ����� � ����� ��� - ���� ����� �������, � ��� ������ �������
}

/**
 * ���������� ������� �� ������
 * @param cmd ��������� �� ������� � ������
 * @return ������������ ��������, ������� ���� ��������� �� ���������� ��������� �������
 */
static uint8_t execute_cmd(char *cmd){
	uint16_t tmp;
	// ���� ������ ��� �������� ����������� ��������
	do{
		// �������� �� ������� � �����������
		if(cmd[0] == 'V'){
			// ����� ����������
			uint8_t n = get_script_digit(cmd+1);
			if(n >= VAR_CNT) break;
			// �������� �������
			tmp = get_script_val(cmd+3);
			if(tmp > 255) break;
			// ������ ��������� �������
			switch(cmd[2]){
			case '=': param.var[n] = tmp; break;
			case '+': tmp += param.var[n]; param.var[n] = tmp > 255 ? 255 : tmp; break;
			case '-': tmp = param.var[n] - tmp; param.var[n] = tmp < 0 ? 255: tmp; break;
			case '*': tmp *= param.var[n]; param.var[n] = tmp > 255 ? 255 : tmp; break;
			case '/': if(tmp == 0) return 0;
					  param.var[n] /= tmp;
					  break;
			case '%': if(tmp == 0) return 0;
					  param.var[n] %= tmp;
					  break;
		    // ���� �� ���������� ��������
			//case '>' param.var[n] = param.var[n] > tmp; break;
			//case '<' param.var[n] = param.var[n] < tmp; break;
			//case '=' param.var[n] = param.var[n] == tmp; break;
			//case '!' param.var[n] = param.var[n] != tmp; break;
			default:
				// ������������ ��������
				break;
			}
			// ����� ������, ������������ �� V, �� �� �������� � ������ - �����
			break;
		}
		// �������� �� ������� � ����������
		if(cmd[2] == '='){
			// �������� ��������
			tmp = get_script_val(cmd+3);
			if(tmp > 255) break;
			// ���� �������� �������� - ��������� �������
			if(strncmp_P(cmd, PSTR("PB"), 2) == 0){
				// �������
				param.bright = tmp;

			} else if(strncmp_P(cmd, PSTR("GB"), 2) == 0){
				// ���������� �������
				for(uint8_t i=0; i<PIXEL_CNT; i++)
					pixels[i].bright = tmp;

			} else if(strncmp_P(cmd, PSTR("PF"), 2) == 0){
				// ���������
				param.fade = tmp;

			} else if(strncmp_P(cmd, PSTR("GF"), 2) == 0){
				// ���������� ���������
				for(uint8_t i=0; i<PIXEL_CNT; i++)
					pixels[i].delta = tmp;

			} else if(strncmp_P(cmd, PSTR("PC"), 2) == 0){
				// ����
				uint16_t tmp2 = get_script_val(cmd+5);
				uint16_t tmp3 = get_script_val(cmd+7);
				if((tmp2 > 255) || (tmp3 > 255)) break;
				param.color.r = tmp;
				param.color.g = tmp2;
				param.color.b = tmp3;

			} else if(strncmp_P(cmd, PSTR("GC"), 2) == 0){
				// ���� ���������
				uint16_t tmp2 = get_script_val(cmd+5);
				uint16_t tmp3 = get_script_val(cmd+7);
				if((tmp2 > 255) || (tmp3 > 255)) break;
				for(uint8_t i=0; i<PIXEL_CNT; i++)
					set_rgb_color(i, tmp, tmp2, tmp3);

			} else if(strncmp_P(cmd, PSTR("PM"), 2) == 0){
				// ����� ��������
				char *ptr = cmd+5;
				while(tmp <= 255){
					if(tmp < PIXEL_CNT){
						if(param.rev) tmp = PIXEL_CNT - tmp - 1;
						// ������ ��� ����������� ������ �������
						pixels[tmp].bright = param.bright;
						pixels[tmp].delta = param.fade;
						pixels[tmp].r = param.color.r;
						pixels[tmp].g = param.color.g;
						pixels[tmp].b = param.color.b;
					} // ������������ ������ �������� ������������ ��� ����������� ��������
					tmp = get_script_val(ptr);
					ptr += 2;
				}

			} else if(strncmp_P(cmd, PSTR("WT"), 2) == 0){
				// ��������
				return tmp;

			}
		}
		// �������� �� ������� ����� �����
		if(strncmp_P(cmd, PSTR("LV"), 2) == 0){
			// ����� ���������� �����
			uint8_t n = get_script_digit(cmd+2);
			if(n > VAR_CNT) break;
			// �������� �������
			if((cmd[3] == '=') || (cmd[3] == '>') || (cmd[3] == '<') || (cmd[3] == '!')){
				tmp = get_script_val(cmd+4);
				if(tmp > 255) break;
				switch(cmd[3]){
				case '=': if(param.var[n] == tmp) goto do_loop_cmd;
						  break;
				case '>': if(param.var[n] > tmp) goto do_loop_cmd;
						  break;
				case '<': if(param.var[n] < tmp) goto do_loop_cmd;
						  break;
				case '!': if(param.var[n] != tmp) goto do_loop_cmd;
						  break;
				}
				goto do_skip_loop;
			}
			// ���� �����������, ���� ���������� �� ����� ����
			if(param.var[n] != 0){
				// ������������� ����������
				param.var[n]--;
	do_loop_cmd:
				// ������� �� ������ �����
				status = script_seek(&script, popstack());
				cmd[0] = 0; // !!!! �����!!!
				if(status == SCRIPT_OK) pushstack(script.pos);
			} else {
				// ���� ���������� ����� �������� - �������� �� ����� ��� ��������,
				// ����� �� �������� ����
	do_skip_loop:
				popstack();
			}

			break;
		}
		// �������� �� ������� ��� ����������
		if(strncmp_P(cmd, PSTR("END"), 3) == 0){
			// ����� �������
			cmd[0] = 0;
			status = SCRIPT_ERROR_IO;
			break;
		} else if(strncmp_P(cmd, PSTR("CLR"), 3) == 0){
			// �������
			memset(pixels, 0, PIXEL_CNT * sizeof(pixel_t));
			memset(&param, 0, sizeof(script_param_t));
			param.bright = 0xFF;

		} else if(strncmp_P(cmd, PSTR("INF"), 3) == 0){
			// ����������� ����
			goto do_loop_cmd;
			//status = script_seek(&script, popstack());
			//cmd[0] = 0; // !!!! �����!!!
			//if(status == SCRIPT_OK) pushstack(script.pos);

		} else if(strncmp_P(cmd, PSTR("RST"), 3) == 0){
			// �������
			status = script_seek(&script, 0);
			cmd[0] = 0; // !!!! �����!!!

		} else if(strncmp_P(cmd, PSTR("RPT"), 3) == 0){
			// ������ �����
			// ����� ���������� !!!!
			uint32_t next = script.pos - (script.readed - (cmd - buf + 3));
			pushstack(next);
			//pushstack(script.pos);

		} else if(strncmp_P(cmd, PSTR("PNT"), 3) == 0){
			// ���������
			return 1;
		}else if(strncmp_P(cmd, PSTR("REV"), 3) == 0){
			// ������
			param.rev = !param.rev;

		} else if(strncmp_P(cmd, PSTR("NEG"), 3) == 0){
			// �������� �������
			for(uint8_t i=0; i<PIXEL_CNT; i++)
				pixels[i].bright ^= 0xFF;
		}
	} while(0);

	return 0;
}

/**
 * ����� ������
 */
static void _start(void){
	uint16_t buf_sz;
	// ������ ����������
	lock_input = 1;
	// ������� �����
	off_all_pixels();
	// ����������� ������ ��� ���
	buf_sz = SD_BUF_SZ;
	buf = get_reserved_memory(&buf_sz);
#if defined(_DEBUG_)
	if(buf_sz != SD_BUF_SZ){
		status = DBG_ERROR;
		return;
	}
#endif
	// ������� ������� �����
	status = sdcard_open();
	if(status == SCRIPT_OK){
		// ������� ������� �� �����
		total_script_files = get_script_count();
		script_id = 0;
		if(total_script_files > 0){
			// ������� ������� ������ ������
			status = script_open(&script, script_id);
		} else
			status = SCRIPT_NOTFOUND;
	}
}

/**
 * ������� ������� � ������
 * @param s ��������� �� ������� � ������
 * @return ��������� �� ������ ����� ������� ������-�����������
 */
static char* skip_cmd(char *s){
	uint8_t n = 0;
	// ������� ������ �� �������� ������
	while(s[n] && (s[n] != ' ') && (s[n] != '\n') && (s[n] != '\r') && (s[n] != '\t')) n++;
	return s + n;
}

/**
 * ������� ���� �������� ������������ � ������
 * @param s ��������� �� ������
 * @return ��������� �� ������ ������ ��������� ������� ��� NULL, ���� ������ ������
 * ���
 */
static char* get_cmd(char *s){
	uint8_t n = 0;
	// ������� ������ �������� ������
	while(s[n] && ((s[n] == ' ') || (s[n] == '\n') || (s[n] == '\r') || (s[n] == '\t'))) n++;
	return s[n] ? s + n : NULL;
}

/**
 * ������ ���� � ���������� ������ �� ����
 * @param buf ����� ������ ����
 * @return ������������ ��������, ������� ���� ���������� ����� ������������
 */
uint8_t script_execute(char *buf){
	uint8_t inf = 0;
	uint8_t result;

	do{
		if(!++inf){
			// ���� ������ ������ 255 �������� ����� - ��� ������ � �������
			status = SCRIPT_INFLOOP;
			break;
		}
		if(script.cmd == NULL){
			// ���� ������� ������� �����������, ���� ��������� ���
			status = script_read_str(&script, buf);
			// ������� ����� � ������ ����
			script.cmd = get_cmd(buf);
			continue;
		}
		// ��������� ������� �������
		result = execute_cmd(script.cmd);
		// � ����� ���������
		script.cmd = get_cmd(skip_cmd(script.cmd));
		// ���� ����� - ����� ��� ���������� ��������
		if(result) return result;
	}while(status == SCRIPT_OK);

	return 0;
}

/**
 * ��������� ���������� ���������� ����� �������
 * @param d ����������� ��������� ������ ����� +1 ��� -1
 * @return ������ PRESET_HIDDEN ��� �������������
 */
static preset_result_t next_file(int8_t d){
	select_next = d;
	return PRESET_NOTHING;
}

/// ������������ ��������� ��������� ������ � ���������� �� 10 ��
#define IND_SD_STATUS		50

/**
 * ���������� ������� � ���� ������� ������� ������ �������� �������
 * @param s ��������� �� ��������� ��������� ������� (������������)
 */
void script_exec(signal_t *s){
	static uint8_t div;

	if(select_next != 0){
		div = 0;
		enter_sd_mode();
		goto next_file_load;
	}

	if(div){
		// ������������� �������� � �������
		if(--div)	return;
	}

	enter_sd_mode();

	switch(status){
	case SCRIPT_OK:
		// ������ ������ �������
		div = script_execute(buf);
		break;
	case SCRIPT_NO_SD:
		div = IND_SD_STATUS;
		_start();
		break;
	case SCRIPT_NOTFOUND:
		div = IND_SD_STATUS;
		_start();
		break;
	case SCRIPT_INFLOOP:
		// ����������� ���� � ������� ����� �������� ������ ������ �����
		// �������� ������ ���������� �������
		div = IND_SD_STATUS;
		status = SCRIPT_ERROR_IO;
		break;
	case SCRIPT_ERROR_IO:
		// ������ ��� ������ ������� - � ���������� ������ ��������� �����
		// ���������� ���� ����� �����, ����� - ���� �����
		if(select_next == 0){
			select_next = 1;
		}
	next_file_load:

		if(total_script_files){
			if(select_next < 0){
				if(script_id == 0)
					script_id = total_script_files-1;
				else
					script_id--;
			} else {
				script_id += select_next;
				if(script_id >= total_script_files) script_id = 0;
			}
		}
		status = script_open(&script, script_id);
		select_next = 0;

	default:
		break;
	}

	leave_sd_mode();
}

/**
 * ���������� ������ ������
 */
static void _stop(void){
	// ������� ����������
	lock_input = 0;
}

/// ������ �������� ��� ��������������� ������� � ���������� �� 10 ��
#define ANIMATE_DELAY		50
#define ANIMATE_LEN			3

/**
 * ����� ���������� � ��������� ������
 * @param show ���� �� ����, �� �������
 * @return ������ INFO_ALWAYS
 */
static effect_info_t _info(uint8_t show){
	static uint8_t ind_pos;
	static uint8_t div = ANIMATE_DELAY;
	if(show){
		char str[17];
		switch(status){
		case SCRIPT_NO_SD:
			center_str_p(1, PSTR("NO CARD"));
			break;
		case SCRIPT_NOTFOUND:
			center_str_p(1, PSTR("NO SCRIPT"));
			break;
		case SCRIPT_ERROR_IO:
			center_str_p(1, PSTR("SEARCH..."));
			break;
		case SCRIPT_OK:
			// �������� ����� ������� �����
			for(uint8_t i=0; i<ANIMATE_LEN; i++){
				str[i] = i == ind_pos ? '>' : '-';
			}
			if(!--div){
				div = ANIMATE_DELAY;
				if(++ind_pos >= ANIMATE_LEN) ind_pos = 0;
			}
			str[ANIMATE_LEN] = ' ';
			str[ANIMATE_LEN+1] = 0;
			// ��� �����
			strncat(str, script.filename, 11);
			show_rpad_str(1, str);
			break;
		case SCRIPT_INFLOOP:
			center_str_p(1, PSTR("INF.LOOP"));
			break;
#if defined(_DEBUG_)
		case DBG_ERROR:
			center_str_p(1, PSTR("<NO MEMORY>"));
			break;
#endif
		}
	}
	return INFO_ALWAYS;
}

/// ��������� �������� ������
static flash_effect_t effect_def = {
	.name = _name,
	.start = _start,
	.work = script_exec,
	.stop = _stop,
	.preset = next_file,
	.save = NULL,
	.info = _info
};

INIT(7){
	// ����������� �������
	register_effect(BACKGROUND_EFFECT, &effect_def);
	// ��� �� ��� ���������� ����������� WS2812b
	DDR(WS_LOCK_PORT) |= WS_LOCK_PIN;

	leave_sd_mode();
}
/**
 * @}
 */

#endif
