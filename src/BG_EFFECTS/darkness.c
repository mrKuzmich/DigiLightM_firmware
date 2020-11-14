/** \file darkness.c
 * \brief \b "����"
 * \n ������ ����������� � ���������� ������� ���������. �� ���� ����� ��� �������,
 * ��� � ���������. ���������� ���������� ������� ���������.
 * \author ARV
 * \date	9 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * \addtogroup BGEFFECTS
 * \copybrief darkness.c ��. \ref darkness.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"
/**
 * @{
 */
static __flash const char _name[] = "DARKNESS";

static void work(signal_t *s){
	for(uint8_t i=0; i<PIXEL_CNT; i++){
		bright_ctrl(i, 0, 0);
		set_rgb_color(i, BLACK);
	}
}

static flash_effect_t effect_def = {
	.name = _name,
	.start = NULL,
	.work = work,
	.stop = NULL,
	.preset = NULL,
	.save = NULL
};

INIT(7){
	// ��������� ������� � ���������

	// ����������� �������
	register_effect(BACKGROUND_EFFECT, &effect_def);
}

/**
 * @}
 */
