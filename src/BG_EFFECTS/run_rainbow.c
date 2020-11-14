/** \file run_rainbow.c
 * \brief \b "������� ������"
 * \n ������ �������������� ������.
 * \author ARV
 * \date	19 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \addtogroup BGEFFECTS
 * \copybrief run_rainbow.c ��. \ref run_rainbow.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"
/**
 * @{
 */

static void _effect(signal_t *s);
static void _stop(void);
static void _start(void);
static preset_result_t _preset(int8_t d);
static void _save(void);

// �������� �������, �� ����� 16 ������
static __flash const char _name[] = "RUNNING RAINBOW";
// �������� �������. ����� �� ������, ����� ��� ����������� ��������
// ������������� ������� �� NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.start	= _start,	// ����� ���� NULL
	.work	= _effect,	// ��� �������� �� ����� ���� NULL!
	.stop	= _stop,	// ����� ���� NULL
	.preset	= _preset,	// ����� ���� NULL
	.save = _save
};

#define PRESET_CNT	5

static int8_t id;
static int16_t first;

typedef struct{
	int8_t	step;
	uint8_t div;
} preset_t;

static __flash const preset_t preset[PRESET_CNT] = {
		{.step = 1, .div=5},
		{.step = 5, .div=5},
		{.step = 10, .div=2},
		{.step = 15, .div=1},
		{.step = 27, .div=1}
};

static EEMEM uint8_t e_id;

INIT(7){
	// ��������� ������� � ���������
	id = eeprom_read_byte(&e_id);
	_preset(0);
	// ����������� ������� - ��������� ������� ��� �������!
	 register_effect(BACKGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// ��������� ���������
	eeprom_update_byte(&e_id, id);
}

static uint8_t br;

// ������� ������� ������� - ���������� 100 ��� � ������� ��� ����������
static void _effect(signal_t *s){
	static uint8_t div = 1;
	hsv_t hsv;
	rgb_t rgb;

	if(--div) return;
	div = preset[id].div;

	if(br < 255) br++;

	hsv.s = hsv.v = 255;
	hsv.h = first;

	for(uint8_t i=0; i<PIXEL_CNT; i++){
		hsv_to_rgb(&hsv, &rgb);
		set_rgb_color(i, rgb.r, rgb.g, rgb.b);
		bright_ctrl(i, br, 1);
		hsv.h += preset[id].step;
		if(hsv.h > HSV_GRADE) hsv.h -= HSV_GRADE;
		if(hsv.h < 0) hsv.h += HSV_GRADE;
	}
	first++;
	first %= HSV_GRADE;
}

// ���������� ������� - ���������� ����� ���, ��� ��������� ������ ������
static void _stop(void){
	// ��������� ����������
}

// ������ ������� - ���������� ����� ����, ��� ���������� ������ ������ �
// � ��� ������, ���� ������� _preset ������� ��������, �������� �� PRESET_NOTHING
static void _start(void){
	// ��������� ����������
	first = rand() % HSV_GRADE;
	br = 1;
}

// ����� ������� ������� - ���������� � ������������ �������
static preset_result_t _preset(int8_t d){
	id += d;
	if(id >= PRESET_CNT) id = 0;
	if(id < 0) id = PRESET_CNT-1;
	return id+1;
}

/**
 * @}
 */
