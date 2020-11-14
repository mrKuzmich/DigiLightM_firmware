/** \file template_effect.c
 * \brief <������� ������� ���������� �� ���� ����� �����>
 * \par
 * <������� ����� ��������� ���������� �� ���� ����� �����>
 * \par \author ARV \par
 * \note <������� ���������� �����>
 * \n �����:
 * \n \date	19 ���. 2017 �.
 * \par
 * \version <������>.	\par
 * Copyright 2015 � ARV. All rights reserved.
 * \par
 * ��� ���������� ���������:\n
 * 	-# AVR GCC 5.2.1 ��� ����� ����� ������
 *
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"

static void _effect(signal_t *s);
static void _stop(void);
static void _start(void);
static void _save(void);
static preset_result_t _preset(int8_t d);

// �������� �������, �� ����� 16 ������
static __flash const char _name[] = "NAME EFFECT";
// �������� �������. ����� �� ������, ����� ��� ����������� ��������
// ������������� ������� �� NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.start	= _start,	// ����� ���� NULL
	.work	= _effect,	// ��� �������� �� ����� ���� NULL!
	.stop	= _stop,	// ����� ���� NULL
	.preset	= _preset,	// ����� ���� NULL
	.save 	= _save		// ����� ���� NULL
};

INIT(7){
	// ��������� ������� � ���������

	// ����������� ������� - ��������� ������� ��� �������!
	// register_effect(BACKGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// ��������� ���������
}

// ������� ������� ������� - ���������� 100 ��� � ������� ��� ����������
static void _effect(signal_t *s){
	// ��������� ����������
}

// ���������� ������� - ���������� ����� ���, ��� ��������� ������ ������
static void _stop(void){
	// ��������� ����������
}

// ������ ������� - ���������� ����� ����, ��� ���������� ������ ������ �
// � ��� ������, ���� ������� _preset ������� ��������, �������� �� PRESET_NOTHING
static void _start(void){
	// ��������� ����������
}

// ����� ������� ������� - ���������� � ������������ �������
static preset_result_t _preset(int8_t d){
	return PRESET_NOTHING;
}

