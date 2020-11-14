/** \file fft_debug.c
 * \brief ���������� ������ "�������"
 *
 * ������ �������� ��������, ����������� ������� �� ��� ��������� ����������
 * ���������� � ������ "��������� �������". �������������� � �������, ��� ������������
 * ������ "�������" ��� ��� ����, ��� � ��� ��������� ������.
 * \author ARV
 * \date	6 ���� 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * ��� ���������� ���������:\n
 * 	-# AVR GCC 5.2.1 ��� ����� ����� ������
 *
 */

#if defined(_FFT_DEBUG_)

#include <avr/io.h>
#include "avr_helper.h"
#include <stdlib.h>
#include "global.h"
#include "fft_debug.h"
#include "lcd_show.h"
#include "lcd.h"
#include "main_effect.h"
#include "spectrum.h"

// ������������ ����, ��� ������������ �� ���
typedef enum{
	ADC_OFFS,
	REAL_OFS,
	VOL_PEAK,
	VOL_AVE,
	F_0,
	F_250,
	F_500,
	F_750,
	F_1000,
	F_1300,
	F_1500,
	F_1800,
	F_2000,
	F_2300,
	F_2500,
	F_2800,
	F_3000,
	F_3300,
	F_3500,
	F_3800,
	B_LF,
	B_MF,
	B_HF,
	//
	SHOW_CNT
} show_t;

// ��������� �������� ��������� ��������
static __flash const char names[SHOW_CNT][9] = {
	"ADC_OFFS",
	"REAL_OFFS",
	"VOL_PEAK",
	"VOL_AVE",
	"F_0",
	"F_250",
	"F_500",
	"F_750",
	"F_1000",
	"F_1300",
	"F_1500",
	"F_1800",
	"F_2000",
	"F_2300",
	"F_2500",
	"F_2800",
	"F_3000",
	"F_3300",
	"F_3500",
	"F_3800",
	"B_LF",
	"B_MF",
	"B_HF"
};

// ������� ������������ �� ���
static show_t item = VOL_PEAK;

// ����������� �����
static void show_val(uint16_t v){
	char str[16];
	utoa(v, str, 10);
	lcd_puts(str);
}

// ������� ��������� �� ��� � ������ �������
void _fft_debug_show(signal_t *s){
	// ���������
	show_rpad_str_p(0, PSTR("DEBUG FFT"));
	// ������������ ��������� ����������
	show_rpad_str_p(1,names[item]);
	lcd_gotoxy(9,1);
	lcd_putc('=');
	// ����� ��������
	switch(item){
	case ADC_OFFS:
		show_val(cfg.dc_offset);
		break;
	case REAL_OFS:
		show_val(real_offset);
		break;
	case VOL_PEAK:
		show_val(s->peak_volume);
		break;
	case VOL_AVE:
		show_val(s->average_vol);
		break;
	case F_0 ... F_3800:
		show_val(s->harmonics[item - F_0]);
		break;
	case B_LF ... B_HF:
		show_val(s->band[item - B_LF]);
		break;
	}
}

// ���� ������� ������������ ��� ������ ������� �������� - ��� ������������

static void _effect(signal_t *s);
static preset_result_t _preset(int8_t d);

static __flash const char _name[] = "@";

static flash_effect_t effect_def = {
	.name	= _name,
	.start	= NULL,	// ����� ���� NULL
	.work	= _effect,	// ��� �������� �� ����� ���� NULL!
	.stop	= NULL,	// ����� ���� NULL
	.preset	= _preset,	// ����� ���� NULL
	.save 	= NULL		// ����� ���� NULL
};

static flash_effect_t effect_def_b = {
	.name	= _name,
	.start	= NULL,	// ����� ���� NULL
	.work	= _effect,	// ��� �������� �� ����� ���� NULL!
	.stop	= NULL,	// ����� ���� NULL
	.preset	= _preset,	// ����� ���� NULL
	.save 	= NULL		// ����� ���� NULL
};

INIT(7){
	register_effect(BACKGROUND_EFFECT, &effect_def_b);
	register_effect(FOREGROUND_EFFECT, &effect_def);
}

// ������� ������� ������� - ���������� 100 ��� � ������� ��� ����������
static void _effect(signal_t *s){
}

// ����� ������� ������� - ���������� � ������������ �������
static preset_result_t _preset(int8_t d){
	item += d;
	if((d < 0) && (item == ADC_OFFS)) item = SHOW_CNT-1;
	if(item >= SHOW_CNT) item = ADC_OFFS;
	return PRESET_HIDDEN;
}

#endif
