/** \file sparkler_effect.c
 * \brief \b "SPARKLER"
 * \n ������ "����������� �����"
 *
 * ������������ ������ ������� ��������,
 * ������� ����� ���������� ����� � ������� ���������� �������� ������ �������,
 * ������� ����� ����.
 *
 * \author ARV
 * \date	15 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * ��� ���������� ���������:\n
 * 	-# AVR GCC 5.2.1 ��� ����� ����� ������
 *
 * \addtogroup FGEFFECT
 * \copybrief sparkler_effect.c ��. \ref sparkler_effect.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"
#include <string.h>
/**
 * @{
 */

static void _effect(signal_t *s);
static void _stop(void);
static void _start(void);
static void _save(void);
static preset_result_t _preset(int8_t d);

// �������� �������, �� ����� 16 ������
static __flash const char _name[] = "SPARKLER";
// �������� �������. ����� �� ������, ����� ��� ����������� ��������
// ������������� ������� �� NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.start	= _start,	// ����� ���� NULL
	.work	= _effect,	// ��� �������� �� ����� ���� NULL!
	.preset	= _preset	// ����� ���� NULL
};

// ���������� ������������ ������� �����, �� �����
#define LIGHT_LIVE_CNT	10

typedef struct{
	uint8_t		pix;		//< ����� �������
	uint8_t		color_id;	//< ���� �������
	uint8_t		live;		//< ����� �����
	uint8_t		speed;		//< �������� �����
	uint8_t		div;		//< ������� ��������
} light_t;

typedef struct{
	uint8_t		live; //< ������� ���������� �����
	uint8_t		h;
	light_t		lights[LIGHT_LIVE_CNT]; //< ������ �����
	uint16_t	prev_peak[F_CNT]; //< ������ ����������� ������
} static_parameter_t;

static static_parameter_t par;

static static_parameter_t *param=&par;

INIT(7){
	// ����������� ������� - ��������� ������� ��� �������!
	register_effect(FOREGROUND_EFFECT, &effect_def);
}


#define MIN_SPEED	5

static uint8_t get_speed(uint16_t ha){
	uint8_t result = MIN_SPEED;
	while(ha && result){
		result--;
		ha >>= 1;
	}
	return result + 1;
}

// ������� ������� ������� - ���������� 100 ��� � ������� ��� ����������
static void _effect(signal_t *s){
	// ������ ����� - ������������ ����� �����
	for(uint8_t i=0; i<LIGHT_LIVE_CNT; i++){
		// ��� ������� ������ �������
		if(param->lights[i].live){
			// ���� ������� ����� �� 0 - ���� ������� �����
			if(pixels[param->lights[i].pix].bright == 0){
				param->live--;
				continue;
			}
			//*/
			// ���� �������� ���������
			if(!--param->lights[i].div){
				// ��������� �������� ��������
				param->lights[i].div = param->lights[i].speed;

				// ��������� ����� �����
				param->lights[i].live--;

				if(++param->lights[i].pix >= PIXEL_CNT){
					// ���� ������� �� �������
					param->lights[i].live = 0;
				}
			}
			// ���� ������� - ��������� ����� ������� �����
			if(param->lights[i].live == 0) param->live--;
		}
	}
	if(1){
		// ������ ������ ����� � ���������� ����� �����
		int16_t delta;

		delta = s->harmonics[param->h] - param->prev_peak[param->h];
		if(delta > 50){
			param->prev_peak[param->h] = s->harmonics[param->h];
			// ���� ��������� ����� � �������
			uint8_t n;
			for(n=0; n<LIGHT_LIVE_CNT; n++){
				if(param->lights[n].live == 0){
					// �����, ��� ����� n
					param->lights[n].speed = get_speed(s->harmonics[param->h] / 10);
					param->lights[n].speed++; // ����� �� ���� 0
					param->lights[n].live = rand() % PIXEL_CNT;
					param->lights[n].color_id = param->h-1;
					param->lights[n].div = param->lights[n].speed;
					param->lights[n].pix = 0;
					if(param->lights[n].live) param->live++;
					break;
				}
			}
		}
		// ��������� �����
		if(param->prev_peak[param->h] >= 5) param->prev_peak[param->h] -= 5;

		param->h += 2;
		if(param->h >= F_CNT) param->h = 1;
		if(param->prev_peak[param->h] >= 5) param->prev_peak[param->h] >>= 1;
	}
	// ������ ������ ��� ����

	off_all_pixels();
	for(uint8_t i=0; i<LIGHT_LIVE_CNT; i++){
		if(param->lights[i].live){
			set_fix_color(param->lights[i].pix, param->lights[i].color_id);
			bright_ctrl(param->lights[i].pix, 255, 10);
		}
	}
}

// ������ ������� - ���������� ����� ����, ��� ���������� ������ ������ �
// � ��� ������, ���� ������� _preset ������� ��������, �������� �� PRESET_NOTHING
static void _start(void){
	memset(param, 0, sizeof(static_parameter_t));
	param->h = 1;
}

// ����� ������� ������� - ���������� � ������������ �������
static preset_result_t _preset(int8_t d){
	return PRESET_NOTHING;
}

/**
 * @}
 */
