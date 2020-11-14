/** \file lamp.c
 * \brief \b "ЛАМПА"
 * \n Постоянная подсветка одним (выбираемым энкодером) цветом. Этот эффект, будучи
 * включен, блокирует реакцию на звуковой сигнал, т.е. этот эффект равносилен
 * включению режима обычного цветного светильника. Только после выбора другого
 * эффекта возобновляется работа в режиме цветомузыки.
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup BGEFFECTS
 * \copybrief lamp.c См. \ref lamp.c
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../hardware.h"
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"
#include "../lcd_show.h"

/**
 * @{
 */
static __flash const char _name[] = "LAMP";

static int16_t color;

static void work(signal_t *s){
	hsv_t hsv;
	rgb_t rgb;

	hsv.h = color;
	hsv.v = 255;
	hsv.s = 255;

	hsv_to_rgb(&hsv, &rgb);

	for(uint8_t i=0; i<PIXEL_CNT; i++){
		bright_ctrl(i, 255, 0);
		set_rgb_color(i, rgb.r, rgb.g, rgb.b);
	}
}

static void _stop(void){
	lock_input = 0;
}

static void _start(void){
	lock_input = 1;
}

static preset_result_t _preset(int8_t d){
	color += d;
	if(color >= HSV_GRADE) color = 0;
	if(color < 0) color = HSV_GRADE-1;
	//return color;
	return 1;
}

#define hex_nibble(x) ((x) >= 0x0A) ? (((x)-0x0A)+'A') : ((x)+'0')

static effect_info_t _info(uint8_t show){
	if(show){
		hsv_t hsv;
		rgb_t rgb;

		hsv.h = color;
		hsv.v = 255;
		hsv.s = 255;

		hsv_to_rgb(&hsv, &rgb);

		char tmp[17] = "COLOR:";
		tmp[6] = hex_nibble(rgb.r >> 4);
		tmp[7] = hex_nibble(rgb.r & 0x0F);
		tmp[8] = hex_nibble(rgb.g >> 4);
		tmp[9] = hex_nibble(rgb.g & 0x0F);
		tmp[10] = hex_nibble(rgb.b >> 4);
		tmp[11] = hex_nibble(rgb.b & 0x0F);
		tmp[12] = 0;

		center_str(1, tmp);
	}
	return INFO_ALWAYS;
}

static void _save(void);

static flash_effect_t effect_def = {
	.name = _name,
	.start = _start,
	.work = work,
	.stop = _stop,
	.preset = _preset,
	.save = _save,
	.info = _info
};

static EEMEM uint16_t e_color;
INIT(7){
	// загрузить пресеты и настройки
	color = eeprom_read_word(&e_color);
	if(color >= HSV_GRADE) color = rand() % HSV_GRADE;

	_preset(0);
	// регистрация эффекта
	register_effect(BACKGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// сохранить настройки
	eeprom_update_word(&e_color, color);
}

/**
 * @}
 */
