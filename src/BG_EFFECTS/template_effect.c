/** \file template_effect.c
 * \brief <введите краткую информацию об этом файле здесь>
 * \par
 * <введите более подробную информацию об этом файле здесь>
 * \par \author ARV \par
 * \note <введите примечания здесь>
 * \n Схема:
 * \n \date	19 апр. 2017 г.
 * \par
 * \version <версия>.	\par
 * Copyright 2015 © ARV. All rights reserved.
 * \par
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <avr/eeprom.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"

static void _effect(signal_t *s);
static void _stop(void);
static void _start(void);
static void _save(void);
static preset_result_t _preset(int8_t d);

// Название эффекта, не более 16 знаков
static __flash const char _name[] = "NAME EFFECT";
// Описание эффекта. Можно не менять, можно для оптимизации заменить
// отсутствующие функции на NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.start	= _start,	// может быть NULL
	.work	= _effect,	// это значение не может быть NULL!
	.stop	= _stop,	// может быть NULL
	.preset	= _preset,	// может быть NULL
	.save 	= _save		// может быть NULL
};

INIT(7){
	// загрузить пресеты и настройки

	// регистрация эффекта - правильно указать тип эффекта!
	// register_effect(BACKGROUND_EFFECT, &effect_def);
}

static void _save(void){
	// сохранить настройки
}

// главная функция эффекта - вызывается 100 раз в секунду для рендеринга
static void _effect(signal_t *s){
	// Требуется реализация
}

// завершение эффекта - вызывается перед тем, как включится другой эффект
static void _stop(void){
	// Требуется реализация
}

// начало эффекта - вызывается после того, как завершился другой эффект и
// в том случае, если функция _preset вернула значение, отличное от PRESET_NOTHING
static void _start(void){
	// Требуется реализация
}

// смена пресета эффекта - вызывается в произвольные моменты
static preset_result_t _preset(int8_t d){
	return PRESET_NOTHING;
}

