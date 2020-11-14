/** \file skystar.c
 * \brief \b "SKY STAR"
 * \n В случайных местах плавно вспыхивают и погасают огни случайных цветов.
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup BGEFFECTS
 * \copybrief skystar.c См. \ref skystar.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"

/// Количество огней во время тишины, не более
#define DEF_EFFECK_BK_LIGHT_CNT	5

static __flash const char _name[] = "SKY STAR";
typedef struct{
	uint8_t speed;
	uint8_t count;
	uint8_t mask_fade;
} preset_t;

#define PRESET_CNT		3

static int8_t preset_id;
static __flash const preset_t preset[PRESET_CNT] = {
		{.speed = 100, .count = 5, .mask_fade = 0x0F},
		{.speed = 5, .count = 10, .mask_fade = 0x1F},
		{.speed = 50, .count = 10, .mask_fade = 0x3F}
};
/**
 * В случайных местах линейки светодиодов зажигаются пикселы случаного цвета.
 * Количество одновременно светящихся пикселей не превышает #DEF_EFFECK_BK_LIGHT_CNT
 * @param s параметры звукового сигнала
 * @return 0, если эффект отрисован (тишина), иначе 1
 */
static void effect(signal_t *s){
	static uint8_t sil_cnt=1;

	rgb_t rgb;
	hsv_t hsv;

	// замедление темпа рисования
	if(--sil_cnt) return;
	sil_cnt = preset[preset_id].speed;

	hsv.h = rand() % HSV_GRADE;
	hsv.s = hsv.v = 255;
	hsv_to_rgb(&hsv, &rgb);

	// вывод в случайном месте пиксела со случайным цветом
	if(get_light_cnt() < preset[preset_id].count)//DEF_EFFECK_BK_LIGHT_CNT)
		rnd_pix_rgb(rgb.r, rgb.g, rgb.b, - (1+(rand() & preset[preset_id].mask_fade)));
}

static preset_result_t _preset(int8_t d){
	preset_id += d;
	if(preset_id < 0) preset_id = PRESET_CNT-1;
	if(preset_id >= PRESET_CNT) preset_id = 0;
	return preset_id+1;
}

static void _save(void);

static flash_effect_t effect_def = {
	.name = _name,
	.start = NULL,
	.work = effect,
	.stop = NULL,
	.preset = _preset,
	.save = _save
};

static EEMEM uint8_t e_preset;

INIT(7){
	// загрузить пресеты и настройки
	preset_id = eeprom_read_byte(&e_preset);
	_preset(0);
	// регистрация эффекта
	register_effect(BACKGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// сохранить настройки
	eeprom_update_byte(&e_preset, preset_id);
}

/**
 * @}
 */
