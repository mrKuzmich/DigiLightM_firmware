/** \file plasma.c
 * \brief \b "PLASMA"
 * \n Ёффект красивый, но очень €ркий. ≈сли заменить установку €ркости с 255 на
 * пиковую громкость, €ркость сильно упадет при том же уровне громкости, но
 * эффект останетс€ при€тным. »зобилует оттенками и при том, что ритмичность
 * рисунка сохран€ет св€зь с мелодией, предсказать внешний вид картинки
 * практически невоможно.
 * \author ARV
 * \date	8 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup FGEFFECTS
 * \copybrief plasma.c —м. \ref plasma.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <stddef.h>
#include <avr/eeprom.h>
#include "../pixel.h"
#include "../global.h"
#include "../main_effect.h"
/**
 * @{
 */

static __flash const char _name[] = "PLASMA";

static uint8_t get_rand(void){
	return  rand() % PIXEL_CNT;
}

/// структура пресета
typedef struct{
	uint16_t	divider;	///< чем больше, тем меньше количество огней
	uint8_t		shift;		///< чем больше, интенсивнее основные цвета
	uint8_t		fade;		///< чем больше, тем медленнее гаснут огни
} preset_t;

#define PRESET_NUM		12
// пресеты: отличаютс€ количеством пикселов, скоростью их погасани€.
static __flash const preset_t preset[PRESET_NUM] = {
	{.divider = 250, .shift = 1, .fade = 15}, //** - basic
	{.divider = 250, .shift = 2, .fade = 15},
	{.divider = 250, .shift = 3, .fade = 15},

	{.divider = 250, .shift = 1, .fade = 75},
	{.divider = 250, .shift = 2, .fade = 75},
	{.divider = 250, .shift = 3, .fade = 75},

	{.divider = 500, .shift = 1, .fade = 15},
	{.divider = 500, .shift = 2, .fade = 15},
	{.divider = 500, .shift = 3, .fade = 15},

	{.divider = 500, .shift = 1, .fade = 75},
	{.divider = 500, .shift = 2, .fade = 75},
	{.divider = 500, .shift = 3, .fade = 75}
};

static int8_t pr_num;

/**
 * ѕЋј«ћј. «ажигаютс€ пикселы в случайных
 * местах.  оличество пикселов красного, синего и зеленого цвета пропорционально
 * уровню соответствующей частотной составл€ющей, но "чистые" цвета не затирают
 * друг друга (если пикселы совпадают), а дополн€ют, что в итоге дает больше
 * белого. Ёффект хорошо смотритс€ на матовом экране при медленных мелоди€х.
 * @param s параметры музыкального сигнала
 * @return всегда 0
 */
static void effect(signal_t *s){
	uint8_t id, _r, _g, _b;
	static uint8_t div =1;

	if(!--div){
		div = 1;
		_r = s->band[LF] / preset[pr_num].divider;
		_g = s->band[MF] / preset[pr_num].divider;
		_b = s->band[HF] / preset[pr_num].divider;

		for(; _r; _r--){
			id = get_rand();
			pixels[id].r = 255;//s->peak_volume;
			if(pixels[id].g) pixels[id].g >>= preset[pr_num].shift;
			if(pixels[id].b) pixels[id].b >>= preset[pr_num].shift;
			bright_ctrl(id, 255, preset[pr_num].fade);
		}

		for(; _g; _g--){
			id = get_rand();
			pixels[id].g = 255;//s->peak_volume;
			if(pixels[id].r) pixels[id].r >>= preset[pr_num].shift;
			if(pixels[id].b) pixels[id].b >>= preset[pr_num].shift;
			bright_ctrl(id, 255, preset[pr_num].fade);
		}

		for(; _b; _b--){
			id = get_rand();
			pixels[id].b = 255;//s->peak_volume;
			if(pixels[id].g) pixels[id].g >>= preset[pr_num].shift;
			if(pixels[id].r) pixels[id].r >>= preset[pr_num].shift;
			bright_ctrl(id, 255, preset[pr_num].fade);
		}
	}
}

static preset_result_t _preset(int8_t d){
	pr_num += d;
	if(pr_num < 0) pr_num = PRESET_NUM-1;
	if(pr_num >= PRESET_NUM) pr_num = 0;
	return pr_num + 1;
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
	pr_num = eeprom_read_byte(&e_preset);
	if((pr_num < 0) || (pr_num >= PRESET_NUM)) pr_num = 0;
	// регистраци€ эффекта
	register_effect(FOREGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// сохранить настройки
	eeprom_update_byte(&e_preset, pr_num);
}

/**
 * @}
 */
