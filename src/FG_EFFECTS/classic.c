/** \file classic.c
 * \brief \b "CLASSIC"
 * \n Классическая цветомузыка: 8 частотных каналов, каждому свой цвет.
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup FGEFFECTS
 * \copybrief classic.c См. \ref classic.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../pixel.h"
#include "../global.h"
#include "../main_effect.h"

/**
 * @{
 */
static __flash const char _name[] = "CLASSIC";
static void _save(void);

#define DIV		5

#define FC					(F_CNT/2)
#define LEN					PIXEL_CNT / (FC)
#define PRESET_CNT			2

typedef struct {
	uint8_t f;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t lvl;
	uint8_t fade1;
	uint8_t fade2;
}preset_f_t;

static int8_t id;
static __flash const preset_f_t preset[PRESET_CNT][FC] = {
	{
			{.f=1,	.r=255,		.g=0,	.b=5,	.lvl=50, .fade1=3, .fade2=2},
			{.f=3,	.r=255,		.g=0,	.b=0,	.lvl=50, .fade1=2, .fade2=2},
			{.f=5,	.r=200,		.g=50,	.b=0,	.lvl=50, .fade1=2, .fade2=2},
			{.f=7,	.r=255,		.g=255,	.b=0,	.lvl=50, .fade1=1, .fade2=2},
			{.f=9,	.r=0,		.g=255,	.b=0,	.lvl=50, .fade1=1, .fade2=2},
			{.f=11,	.r=0,		.g=255,	.b=50,	.lvl=50, .fade1=1, .fade2=2},
			{.f=13,	.r=0,		.g=255,	.b=255,	.lvl=50, .fade1=1, .fade2=2},
			{.f=15,	.r=0,		.g=0,	.b=255,	.lvl=50, .fade1=1, .fade2=2}
	},
	{
			{.f=1,	.r=255,		.g=0,	.b=0,	.lvl=70, .fade1=3, .fade2=1},
			{.f=3,	.r=255,		.g=0,	.b=0,	.lvl=70, .fade1=2, .fade2=1},
			{.f=5,	.r=255,		.g=0,	.b=0,	.lvl=70, .fade1=2, .fade2=1},
			{.f=7,	.r=0,		.g=255,	.b=0,	.lvl=50, .fade1=1, .fade2=2},
			{.f=9,	.r=0,		.g=255,	.b=0,	.lvl=50, .fade1=1, .fade2=2},
			{.f=11,	.r=0,		.g=0,	.b=255,	.lvl=40, .fade1=1, .fade2=3},
			{.f=13,	.r=0,		.g=0,	.b=255,	.lvl=40, .fade1=2, .fade2=3},
			{.f=15,	.r=0,		.g=0,	.b=255,	.lvl=40, .fade1=3, .fade2=3}
	}
};

static uint8_t bright_f(uint8_t v){
	uint16_t br = v * 2;
	if(br > 255) br = 255;
	return br;
}

/**
 * Классический эффект, когда каждой частотной полосе сопоставлена яркость
 * определенного цвета. В данном случае 8 цветов. Динамичный эффект-мигалка.
 * @param s параметры музыкального сигнала
 * @return всегда 0
 */
static void effect(signal_t *s){
	static uint8_t div = DIV;
	uint8_t br, px=0;

	if(--div) return;
	div = DIV;

	for(uint8_t f=0; f < FC; f++){
		br = (s->harmonics[preset[id][f].f] >> 1) > preset[id][f].lvl ? bright_f(s->peak_volume) : pixels[px+1].bright>>preset[id][f].fade1;
		for(uint8_t i=0; i<LEN; i++){
			bright_ctrl(px+i, br, preset[id][f].fade2);
			set_rgb_color(px+i, preset[id][f].r, preset[id][f].g, preset[id][f].b);
		}
		px += LEN;
	}

/*
	while(f <= F_3800_HZ){
		for(uint8_t i=0; i<4; i++){
			bright_ctrl(px+i, (s->harmonics[f] >> 1) > 50 ? bright_f(s->peak_volume) : pixels[px+1].bright>>1, 2);
			set_fix_color(px+i, f/2);
		}
		px += 4;
		f += 2;
	}
*/
}

static preset_result_t _preset(int8_t d){
	id += d;
	if(id < 0) id = PRESET_CNT-1;
	if(id >= PRESET_CNT) id = 0;
	return id+1;
}

static flash_effect_t effect_def = {
	.name = _name,
	.start = NULL,
	.work = effect,
	.stop = NULL,
	.preset = _preset,
	.save	= _save
};

static EEMEM uint8_t e_id;

INIT(7){
	// загрузить пресеты и настройки
	id = eeprom_read_byte(&e_id);
	_preset(0);
	// регистрация эффекта
	register_effect(FOREGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// сохранить настройки
	eeprom_update_byte(&e_id, id);
}

/**
 * @}
 */
