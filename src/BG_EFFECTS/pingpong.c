/** \file pingpong.c
 * \brief \b "PING-PONG"
 * \n ќгонек измен€ющегос€ цвета бегает от кра€ к краю цепочки светодиодов.
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup BGEFFECTS
 * \copybrief pingpong.c —м. \ref pingpong.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../global.h"
#include "../pixel.h"
#include "../color_transform.h"
#include "../main_effect.h"
/**
 * @{
 */
static __flash const char _name[] = "PING-PONG";

typedef struct{
	uint8_t speed;
	uint8_t fade;
	uint8_t color_speed;
} preset_t;

#define PRESET_CNT		5

static int8_t preset_id;
static __flash const preset_t preset[PRESET_CNT] = {
		{.speed = 2, .fade = 10, .color_speed = 1},
		{.speed = 2, .fade = 1, .color_speed = 10},
		{.speed = 5, .fade = 10, .color_speed = 1},
		{.speed = 5, .fade = 1, .color_speed = 10},
		{.speed = 5, .fade = 1, .color_speed = 0}
};

/**
 * ќт кра€ к краю полоски светодиодов перемещаетс€ пиксель максимальной €ркости
 * и плавно мен€ющимс€ "по радуге" цветом.
 * @param s параметры звукового сигнала
 * @return 0, если эффект отрисован (тишина), 1 в противом случае
 */
static void effect(signal_t *s){
	static int16_t h1;
	static uint8_t px;
	static int8_t dx=1;
	static uint8_t sil_cnt=1;
	hsv_t hsv;
	rgb_t rgb;

	hsv.s = 255;
	hsv.v = 255;
	if(--sil_cnt) return;
	sil_cnt = preset[preset_id].speed;

	hsv.h = h1;
	hsv_to_rgb(&hsv, &rgb);

	for(uint8_t i=0; i < PIXEL_CNT; i++){
		pixels[i].bright >>=1;
	}
	set_rgb_color(px, rgb.r, rgb.g, rgb.b);
	bright_ctrl(px, 255, preset[preset_id].fade);
	px += dx;
	if(px >= PIXEL_CNT){
		px -= dx;
		dx = -dx;
		if(preset[preset_id].color_speed == 0){
			h1 = rand() % HSV_GRADE;
		} else {
			h1 += preset[preset_id].color_speed;
			if(h1 >= HSV_GRADE) h1 -= HSV_GRADE;
		}
	}
}

static preset_result_t _preset(int8_t d){
	preset_id += d;
	if(preset_id < 0) preset_id = PRESET_CNT-1;
	if(preset_id >= PRESET_CNT) preset_id = 0;
	return preset_id + 1;
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
	// регистраци€ эффекта
	register_effect(BACKGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// сохранить настройки
	eeprom_update_byte(&e_preset, preset_id);
}

/**
 * @}
 */
