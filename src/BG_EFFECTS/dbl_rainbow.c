/** \file dbl_rainbow.c
 * \brief \b "DOUBLE RAINBOW"
 * \n ƒва посто€нно мен€ющихс€ цвета затухают от краев ленты светодиодов к ее
 * центру.
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup BGEFFECTS
 * \copybrief dbl_rainbow.c —м. \ref dbl_rainbow.c
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
static int16_t h1, h2;

static void ini(void){
	int8_t dl = 255 / (PIXEL_CNT / 2);
	int16_t h = 255;
	for(uint8_t i=0; i < PIXEL_CNT; i++){
		set_rgb_color(i, 0,0,0);
		bright_ctrl(i, h, 0);
		h -= dl;
		if(h < 0){
			dl = -dl;
			h -= dl;
		}
	}
	h1 = rand() % HSV_GRADE;
	h2 = rand() % HSV_GRADE;
}

typedef struct{
	int8_t speed1;
	int8_t speed2;
} preset_t;

#define PRESET_CNT	4
static int8_t id;
static __flash const preset_t preset[PRESET_CNT] = {
		{.speed1 = 20, .speed2 = 20},
		{.speed1 = 5, .speed2 = 20},
		{.speed1 = 20, .speed2 = 5},
		{.speed1 = 5, .speed2 = 5}
};

/**
 * ќт краев линейки светодиодов к центру плавно затухают различные оттенки
 * цвета. ÷вета краев плавно мен€ютс€ в "противоположном" направлении по
 * кругу HSV-модели, т.е. в "радужной" последовательности.
 * @param s параметры звукового сигнала
 * @return 0, если эффект отрисован (тишина), 1 в противном случае
 */
static void effect(signal_t *s){
	static uint8_t d1=1, d2=1;
	hsv_t hsv;
	rgb_t rgb1, rgb2;
	if(pixels[0].bright == 0) ini();

	hsv.s = 255;
	hsv.v = 255;

	if(!--d1){
		d1 = preset[id].speed1;
		h1++;
		if(h1 >= HSV_GRADE) h1 = 0;
	}
	if(!--d2){
		d2 = preset[id].speed2;
		h2--;
		if(h2 < 0) h2 = HSV_GRADE-1;
	}
	hsv.h = h1;
	hsv_to_rgb(&hsv, &rgb1);
	hsv.h = h2;
	hsv_to_rgb(&hsv, &rgb2);
	// во врем€ тишины рисуем фоновый рисунок
	for(uint8_t i=0; i < PIXEL_CNT; i++){
		if(i < (PIXEL_CNT/2))
			set_rgb_color(i, rgb1.r, rgb1.g, rgb1.b);
		else
			set_rgb_color(i, rgb2.r, rgb2.g, rgb2.b);
	}
}

static __flash const char _name[] = "DOUBLE SPECTRE";

static preset_result_t _preset(int8_t d){
	id += d;
	if(id >= PRESET_CNT) id = 0;
	if(id < 0) id = PRESET_CNT-1;
	return id+1;
}

static EEMEM uint8_t e_id;

static void _save(void){
	// сохранить настройки
	eeprom_update_byte(&e_id, id);
}

static flash_effect_t effect_def = {
	.name = _name,
	.start = ini,
	.work = effect,
	.stop = NULL,
	.preset = _preset,
	.save = _save
};

INIT(7){
	// загрузить пресеты и настройки
	id = eeprom_read_byte(&e_id);
	_preset(0);
	// регистраци€ эффекта
	register_effect(BACKGROUND_EFFECT, &effect_def);
}

/**
 * @}
 */
