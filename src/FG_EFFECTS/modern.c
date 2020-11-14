/** \file modern.c
 * \brief \b "MODERN"
 * \n Эффект, реализующий яркость цвета количеством светящихся пикселов, т.е.
 * шириной полоски. Полоска вспыхивает на полную и затем постепенно схлопывается.
 *
 * \author ARV
 * \date	19 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 * \addtogroup FGEFFECT
 * \copybrief modern.c См. \ref modern.c
 *
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"
#include "../lcd_show.h"
/**
 * @{
 */

static void _effect(signal_t *s);
static void _start(void);
static preset_result_t _preset(int8_t d);
static effect_info_t _info(uint8_t show);

// Название эффекта, не более 16 знаков
static __flash const char _name[] = "MODERN";
// Описание эффекта. Можно не менять, можно для оптимизации заменить
// отсутствующие функции на NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.start	= _start,	// может быть NULL
	.work	= _effect,	// это значение не может быть NULL!
	.preset	= _preset,	// может быть NULL
	.info	= _info
};

// кол-во цветных каналов
#define CHANEL_CNT		6

static const __flash  uint8_t chanel[CHANEL_CNT] = {
		F_DC, F_250_HZ, F_1500_HZ, F_2300_HZ, F_3300_HZ, F_3800_HZ};

static __flash const rgb_t color[CHANEL_CNT] = {
		{0xFF,0x0F,0x00},{RED},{GREEN},{ORANGE},{BLUE},{PURPLE},{RED},{YELLOW},{GREEN},{ORANGE},{BLUE},{PURPLE},{RED},{YELLOW},{GREEN},{ORANGE}
};

typedef struct{
	uint16_t	level;
	uint8_t		wait;
	uint8_t		age;
} chanel_t;

typedef struct {
	int8_t		preset;
	uint8_t		DIV;
	uint8_t		div;
	chanel_t	chanel[CHANEL_CNT];
} param_t;

static param_t par;
static param_t *param=&par;

#define CHANEL_WIDTH	(PIXEL_CNT / CHANEL_CNT)

INIT(7){
	// регистрация эффекта - правильно указать тип эффекта!
	register_effect(FOREGROUND_EFFECT, &effect_def);
}

static effect_info_t _info(uint8_t show){
	if(show){
		center_str_p(1, PSTR("6 CHANELS"));
	}
	return INFO_ONCE;
}

#define MAX_AGE		10

// начало эффекта - вызывается после того, как завершился другой эффект и
// в том случае, если функция _preset вернула значение, отличное от PRESET_NOTHING
static void _start(void){
	param->preset = 0;
	param->DIV = 3;
	param->div = param->DIV;
	for(uint8_t i=0; i<CHANEL_CNT; i++)
		param->chanel[i].age = MAX_AGE;
}

// смена пресета эффекта - вызывается в произвольные моменты
static preset_result_t _preset(int8_t d){
	return PRESET_NOTHING;
}

#define WAIT	2
#define LVL_DEC	30

// главная функция эффекта - вызывается 100 раз в секунду для рендеринга
static void _effect(signal_t *s){
	// предделение
	if(param->div){
		param->div--;
		return;
	}
	param->div = param->DIV;
	off_all_pixels();

	uint16_t d;
	uint8_t w = CHANEL_WIDTH;
	uint16_t k;

	// определение пиков
	for(uint8_t i=0; i<CHANEL_CNT; i++){
		d = s->harmonics[chanel[i]];
		k = param->chanel[i].level;
		if((d > k)){
			param->chanel[i].level = d;
			param->chanel[i].wait = WAIT;
			d = d - k;
			k = CHANEL_WIDTH/2;
			while(d && k){
				d >>= 1;
				k--;

			}
			param->chanel[i].age = k;

		}
		switch(param->preset){
		case 0:
			for(uint8_t p=0; p<w; p++){
				//if(i==0)
					k = -(CHANEL_WIDTH/2);
				if((p >= param->chanel[i].age) && (p < (w - param->chanel[i].age))){
					k += p+w*i;
					if(k >= 0){
						set_rgb_color(k, color[i].r, color[i].g, color[i].b);
						bright_ctrl(k, 255, 30);
					}
				}
			}
			break;
		}
		if(param->chanel[i].wait){
			param->chanel[i].wait--;
		} else {
			if(param->chanel[i].level >= LVL_DEC)
				param->chanel[i].level -= LVL_DEC;
			else
				param->chanel[i].level = 0;
			if(param->chanel[i].age <= MAX_AGE)
				param->chanel[i].age++;
		}
	}
}

/**
 * }@
 */
