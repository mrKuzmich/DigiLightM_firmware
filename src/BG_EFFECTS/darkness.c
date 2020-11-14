/** \file darkness.c
 * \brief \b "ТЬМА"
 * \n Эффект заключается в отсутствии фоновой подсветки. То есть когда нет сигнала,
 * нет и подсветки. Эквивалент отключения фоновой подсветки.
 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * \addtogroup BGEFFECTS
 * \copybrief darkness.c См. \ref darkness.c
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
	// загрузить пресеты и настройки

	// регистрация эффекта
	register_effect(BACKGROUND_EFFECT, &effect_def);
}

/**
 * @}
 */
