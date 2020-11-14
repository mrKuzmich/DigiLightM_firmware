/** \file main_effect.c
 * \brief ��������� ��� �������� ����������
 *
 * ������ ��������� ����������� ��������� � ��������� �������, �����������
 * ������ ������������� ���������� ������� �������� �� ��������� ���������.
 * \author ARV
 * \date	18 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include "avr_helper.h"
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "pixel.h"
#include "main_effect.h"
#include "gain_ctrl.h"
#include "spectrum.h"
#include "lcd_show.h"
#include "sd_script.h"

/// ��������� �������� ������� ��������
typedef struct{
	int8_t			cur;	///< ����� �������� ���������� ������
	int8_t			total;	///< ����� ���������� ��������
	uint8_t			first;	///< ������� ������� ������� �������
	flash_effect_t	*effect[MAX_EFFECT_CNT];///< ������ ���������� �� �������
} effect_state_t;

/// ������ �������� �� �����
static effect_state_t effects[2];

/**
 * �������� ������ � ������ ���������������� ����, ����� ��� ���������
 * \note ���� ������ ��������, ������ �� �������������� ��� ������-���� �����������.
 * @param toe ��� ������� #BACKGROUND_EFFECT ��� #FOREGROUND_EFFECT
 * @param eff ��������� �� ��������� � ��������� �������
 */
void register_effect(uint8_t toe, flash_effect_t *eff){
#if defined(_FFT_DEBUG_)
	if(eff->name[0] != '@') return;
#endif
	if(effects[toe].total >= MAX_EFFECT_CNT) return;
	effects[toe].effect[effects[toe].total] = eff;
	effects[toe].total++;
}

/// ������� ������ � ������� ������� ��������
#define BACK	 effects[BACKGROUND_EFFECT]
/// ������� ������ � ������� �������� ��������
#define FORE	 effects[FOREGROUND_EFFECT]
/// ������� ������ � �������� �������� �������
#define CUR_BACK effects[BACKGROUND_EFFECT].effect[effects[BACKGROUND_EFFECT].cur]
/// ������� ������ � �������� ��������� �������
#define CUR_FORE effects[FOREGROUND_EFFECT].effect[effects[FOREGROUND_EFFECT].cur]

/**
 * �������� �� ������ ������� ��� ������� � ��������� ��� ������
 * @param s ��������� ��������� �������
 */
void execute_effect(signal_t *s){
	sens_control(s);
	if(s->average_vol > DEF_SILENCE_LVL){
		// foreground
		if(FORE.first){
			if(CUR_BACK->stop != NULL) CUR_BACK->stop();
			if(CUR_FORE->start != NULL)CUR_FORE->start();
			FORE.first = 0;
		}
		CUR_FORE->work(s);
		BACK.first = 1;
	} else {
		// background
		if(BACK.first){
			if(CUR_FORE->stop != NULL) CUR_FORE->stop();
			if(CUR_BACK->start != NULL) CUR_BACK->start();
			BACK.first = 0;
		}
		CUR_BACK->work(s);
		FORE.first = 1;
	}
}

/**
 * ���� ������� ������ �������� ������� ����� ��������, �� �������� ��.
 * @param s ��������� ��������� �������
 * @param delta ����������� ����� �������
 * @return #PRESET_NOTHING, ���� �������� ���, #PRESET_HIDDEN, ���� �����
 * �������� �� ������ �������������� �� ���, ��� ����� ������ ���������� �������
 * � ��������� ������
 */
int8_t change_preset(signal_t *s, int8_t delta){
	preset_result_t result = PRESET_NOTHING;
	flash_effect_t *ptr = (s->average_vol > DEF_SILENCE_LVL) ? CUR_FORE : CUR_BACK;
	effect_state_t *st = (s->average_vol > DEF_SILENCE_LVL) ? &FORE : &BACK;

	if(ptr->preset != NULL) result = ptr->preset(delta);

	if(result != PRESET_NOTHING) st->first = 1;
	return result;
}

static char tmp[17];

/**
 * ���������� �������� �������� ��������� �������
 * @param s ��������� ��������� �������
 * @return ��������� �� ������ �� FLASH � ���������
 */
char* get_effect_name(signal_t *s){
	strcpy_P(tmp, (s->average_vol > DEF_SILENCE_LVL) ? CUR_FORE->name : CUR_BACK->name);
	return tmp;
}

static char* get_preset_name(signal_t *s){
	preset_result_t _preset = PRESET_NOTHING;
	flash_effect_t *ptr = (s->average_vol > DEF_SILENCE_LVL) ? CUR_FORE : CUR_BACK;

	if(ptr->preset != NULL)	_preset = ptr->preset(0);

	switch(_preset){
	case PRESET_HIDDEN:
	case PRESET_NOTHING:
		tmp[0] = 0;
		break;
	default:
		// ���� ��� ������ ������������ �������
		strcpy_P(tmp, PSTR("PRESET "));
		itoa(_preset, &tmp[7], 10);
	}
	return tmp;
}

void show_info(signal_t *s){
	flash_effect_t *ptr = (s->average_vol > DEF_SILENCE_LVL) ? CUR_FORE : CUR_BACK;

	if(ptr->info != NULL){
		ptr->info(1);
	} else {
		center_str(1, get_preset_name(s));
	}
}

uint8_t effect_auto_changed(signal_t *s){
	static uint8_t mode = 1;
	uint8_t changed = 0;

	flash_effect_t *ptr = s->average_vol > DEF_SILENCE_LVL ? CUR_FORE : CUR_BACK;
	if(ptr->info != NULL) changed = ptr->info(0) == INFO_ALWAYS;

	uint8_t cur_mode = s->average_vol > DEF_SILENCE_LVL;

	changed = changed || (cur_mode != mode);
	mode = cur_mode;
	return changed;
}

/**
 * ���������� ������ ������� �� ��������� - �������� ������� �������� ��������
 */
static void default_stop_effect(void){
	for(uint8_t i=0; i<PIXEL_CNT; i++){
		pixels[i].delta = 10;
	}
}

static EEMEM int8_t e_cur_back;
static EEMEM int8_t e_cur_fore;

void select_fg_effect(signal_t *s, uint8_t num){
	if(s->average_vol > DEF_SILENCE_LVL){
		if(num < FORE.total){
			if(CUR_FORE->stop != NULL) CUR_FORE->stop();
			default_stop_effect();
			FORE.cur = num;
			FORE.first = 1;
			eeprom_update_byte((void*)&e_cur_fore, FORE.cur);
		}
	}
}

void select_bg_effect(signal_t *s, uint8_t num){
	if(s->average_vol <= DEF_SILENCE_LVL){
		if(num < BACK.total){
			if(CUR_BACK->stop != NULL) CUR_BACK->stop();
			default_stop_effect();
			BACK.cur = num;
			BACK.first = 1;
			eeprom_update_byte((void*)&e_cur_back, BACK.cur);
		}
	}
}


/**
 * ��������� � EEPROM ��������� ���� ��������.
 */
void save_all_effects(void){
	eeprom_update_byte((void*)&e_cur_fore, FORE.cur);
	eeprom_update_byte((void*)&e_cur_back, BACK.cur);
	for(uint8_t i = 0; i < FORE.total; i++){
		if(FORE.effect[i]->save != NULL)
			FORE.effect[i]->save();
	}
	for(uint8_t i = 0; i < BACK.total; i++){
		if(BACK.effect[i]->save != NULL)
			BACK.effect[i]->save();
	}
}

/**
 * ���������� ������ ������ �� ������� � ������������ � ������� ������� �������.
 * @param s ��������� ��������� �������
 * @param delta ����������� ����� �������
 */
void change_effect(signal_t *s, int8_t delta){
	flash_effect_t *ptr = s->average_vol > DEF_SILENCE_LVL ? CUR_FORE : CUR_BACK;
	effect_state_t *st = s->average_vol > DEF_SILENCE_LVL ? &FORE : &BACK;

	if(ptr->stop != NULL)	ptr->stop();

	default_stop_effect();

	st->cur += delta;
	if(st->cur < 0) st->cur = st->total-1;
	if(st->cur >= st->total) st->cur = 0;

	ptr = s->average_vol > DEF_SILENCE_LVL ? CUR_FORE : CUR_BACK;
	st->first = 1;
	eeprom_update_byte((void*)&e_cur_fore, FORE.cur);
	eeprom_update_byte((void*)&e_cur_back, BACK.cur);
}

INIT(8){
	FORE.cur = eeprom_read_byte((void*)&e_cur_fore);
	if((FORE.cur < 0) || (FORE.cur >= FORE.total)) FORE.cur = 0;
	BACK.cur = eeprom_read_byte((void*)&e_cur_back);
	if((BACK.cur < 0) || (BACK.cur >= BACK.total)) BACK.cur = 0;
}
