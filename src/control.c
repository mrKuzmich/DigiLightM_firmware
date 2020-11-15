/** \file control.c
 * \brief ���������� � ���������
 * ������, �������������� ��������� �������� � ������� �������� � ������, �
 * ��� �� ����������� ����� ���������� �� ���.

 * \author ARV
 * \date	9 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * \addtogroup CONTROL
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "power_control.h"
#include <string.h>
#include <stdlib.h>
#include "gain_ctrl.h"
#include "encoder.h"
#include "lcd.h"

#include "pixel.h"
#include "control.h"
#include "global.h"
#include "main_effect.h"
#include "lcd_show.h"
#include "ir_remote.h"
#include "menu.h"
#include "setup.h"

#if defined(_FFT_DEBUG_)
#include "fft_debug.h"
#endif

/**
 * ��������� ����������� ���������� � ������������ ��������.
 * @param v ���������� ����������
 * @param lim �������������� ������� ��������
 * @param d ��� ��������� ��������
 */
static void change_val(int8_t *v, uint8_t lim, int8_t d){
	int16_t tmp = *v;
	tmp += d;
	if(tmp >= lim) tmp = lim-1;
	if(tmp < 0) tmp = 0;
	*v = tmp;
}

/// ����� �������� ��������� ������ ������
enum button_state{
	BTN_WAIT_RELEASED,	///< �������� ���������� ������
	BTN_WAIT_PRESSED,	///< �������� ������� ������
	BTN_WAIT_DEBOUNCE	///< ���������� ��������
};

/// ��������������� ��� ��� �������� ��������� ������ ������
typedef struct{
	uint8_t state : 2;	///< ��������� ��������
	uint8_t div	  : 1;	///< ���� ���������� ����� ������
} btn_state_t;

/**
 * ����������� ������ ����������������
 * @param delta ����������� ��������� ������
 */
static void change_level(int8_t delta){
	if(cfg.agc_enabled)
		return; // ��� ��������� ��� ������ ������������� �� ��������
	int8_t lvl = cfg.sensitivity[cfg.input];
	change_val(&lvl, MAX_LEVEL, delta);
	cfg.sensitivity[cfg.input] = lvl;
}

/**
 * ��������� ������
 * @return ������� �������� ������� ������
 */
uint8_t get_btn_state(void){
	return ~PIN(BTN_PORT) & (BTN_CHANGE | BTN_MENU);
}

// ������� ���������� ������ ������� ����, �.�. ��������� "�� �������"
static uint8_t in_menu;

/**
 * ��������� ���� ������� ������
 * \note ���������� �������� �������������� ��� ������ �������� ���������,
 * �.�. ������� ������ ���� ������� ������� ������ ��� ����������� �������.
 * @return ��� ������
 */
static uint8_t get_pressed_btn(void){
	static btn_state_t st = {.state = BTN_WAIT_RELEASED};
	static uint8_t old;
	uint8_t btn = get_btn_state();

	switch(st.state){
	case BTN_WAIT_RELEASED:
		if(!btn){
			st.state = BTN_WAIT_PRESSED;
			st.div = 1;
			old = 0;
		}
		break;
	case BTN_WAIT_PRESSED:
		if(btn){
			old = btn;
			st.div = 1;
			st.state = BTN_WAIT_DEBOUNCE;
		}
		break;
	case BTN_WAIT_DEBOUNCE:
		if(st.div){
			st.div = 0;
			break;
		}
		if(old == btn){
			st.state = BTN_WAIT_RELEASED;
			return btn;
		}
	}
	return BTN_NONE;
}

/**
 * ��������� ������� ���������� �� ���������� ������� ����������
 * TODO ����������� ������� ���������� ������� ��� ���������� ������� ������
 * (�������� ��� ����?)
 * @return
 */
control_t get_control(void){
	// ������� ������
	uint8_t btn = get_pressed_btn();
	if(btn != BTN_NONE){
		switch(btn){
		case BTN_CHANGE:
			return CTRL_ENC_BTN;
		case BTN_MENU:
			return CTRL_BTN_SET;
		}
	}
	// ����� �������
	int8_t enc = get_encoder();
	if(enc != 0){
		if(enc > 0)
			return btn == BTN_CHANGE ? CTRL_ENC_BTN_UP : CTRL_ENC_UP;
		else
			return btn == BTN_CHANGE ? CTRL_ENC_BTN_DN : CTRL_ENC_DN;
	} else {
		if(btn == BTN_CHANGE) return CTRL_BTN_SET;
	}

	return CTRL_NONE;
}

/**
 * ��������� ������� ���������� �� ���� ������� ����������, ������� ����� � ��.
 * @param ctrl ��� �� ���������� ������� ����������
 * @return ������� ����������
 * \note ���������� ������ ���������� ����� ��������� ��� "��������"
 */
static control_cmd_t get_command(control_t ctrl){
	// ������� ��������� ���������� ������� ����������
	switch(ctrl){
	case CTRL_BTN_PWR:
		return CMD_POWER;
	case CTRL_BTN_SET:
		return CMD_SETUP;
	case CTRL_ENC_BTN_UP:
	case CTRL_ENC_BTN:
		return CMD_EFFECT_PLUS;
	case CTRL_ENC_BTN_DN:
		return CMD_EFFECT_MINUS;
	case CTRL_ENC_UP:
		return CMD_PRESET_PLUS;
	case CTRL_ENC_DN:
		return CMD_PRESET_MINUS;
	default:
		break;
	}
	// TODO �������� ���� ������ �� UART
	// � ����� - ����� IR
	return get_ir_command();
}

// ��������������� ��� ������ ���������
typedef enum{
	EXEC_NO_SHOW,	// ������ �� ��������� �� ���
	EXEC_SHOW,		// �������� ��� � ������� ������
	EXEC_SCALE		// �������� ����� ���������������� �� ���
} exec_result_t;

/**
 * ���������� ������� ����������
 * @param s ��������� ��������� �������
 * @return ������ ���������� ���������
 */
static exec_result_t exec_cmd(signal_t *s){
	control_cmd_t cmd = get_command(get_control());
	exec_result_t result = EXEC_SHOW;

	switch(cmd){
	case CMD_POWER:
		power_off();
		break;
	case CMD_VOLUME_PLUS:
		change_level(1);
		result = EXEC_SCALE;
		break;
	case CMD_VOLUME_MINUS:
		change_level(-1);
		result = EXEC_SCALE;
		break;
	case CMD_EFFECT_PLUS:
		change_effect(s, 1);
		break;
	case CMD_EFFECT_MINUS:
		change_effect(s, -1);
#if !defined(ONLY_BASE_CMD)
	case CMD_EFFECT_1 ... CMD_EFFECT_10:
		// TODO ����� ����������� �������
		//select_fg_effect(s, cmd - CMD_EFFECT_1);
		//select_bg_effect(s, cmd - CMD_PRESET_1);
		break;
#endif
	case CMD_PRESET_PLUS:
		change_preset(s, 1);
		break;
	case CMD_PRESET_MINUS:
		change_preset(s, -1);
		break;
#if !defined(ONLY_BASE_CMD)
	case CMD_PRESET_1 ... CMD_PRESET_10:
		// TODO ����� ����������� �������
		break;
#endif
	case CMD_SETUP:
		in_menu = 1;
		break;
	default:
		result = EXEC_NO_SHOW;
	}

	return result;
}

/**
 * ������������ ������� ������ � ��������� ��������������� ��������,
 * ��������� ���, ���� �� ����, ��������� ������� ��� � �.�.
 * @param s ��������� ��������� �������
 */
void do_control(signal_t *s){
	static uint8_t timeout;
	uint8_t do_show = 0;
	exec_result_t exer;

	if(in_menu && cfg.lcd_enabled){
		// ������������ ������� ����
		switch(do_menu()){
		case MENU_CONTINUE:
			// ����������� ����
			return;
		case MENU_DONE:
			// ����� �� ����
			in_menu = 0;
			do_show = 1;
			break;
		case MENU_DONE_MSG:
			// ����� �� ���� � ����������
			in_menu = 0;
			timeout = IND_TIMEOUT;
			break;
		}
	}

	// �������� � ������������ �������
	exer = exec_cmd(s);
	if(!do_show)
		do_show = exer != EXEC_NO_SHOW;
	// ���� ��� ����� �����������, ���������� ������� �������������� �������
	if(do_show)	reset_power_timeout();

#if !defined(_FFT_DEBUG_)
	do_show = do_show || effect_auto_changed(s);

	if(cfg.lcd_enabled){
		if(do_show || !timeout){
			if(exer == EXEC_SCALE){
				show_rpad_str_p(0, VOL_NAME);
				show_scale(1, cfg.sensitivity[cfg.input]);
			} else {
				center_str(0, get_effect_name(s));
				//preset_name = get_preset_name(s);
				if(do_show)	//center_str(1, preset_name);
					show_info(s);
				else
					show_spectrum(s);
			}
			timeout = (do_show) ? IND_TIMEOUT : 0;
		}
		if(timeout) timeout--;
	}
#else
	_fft_debug_show(s);
#endif
}

/// ������������� ������
INIT(7){
	// ������
	DDR(BTN_PORT) &= ~BTN_ANY;
	PORT(BTN_PORT) |= BTN_ANY;
}

/**
 * @}
 */
