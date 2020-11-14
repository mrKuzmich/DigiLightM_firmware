/** \file sparkler_effect.c
 * \brief \b "SPARKLER"
 * \n Эффект "Бенгальский огонь"
 *
 * Длительность полета огонька случайна,
 * разница между предыдущим пиком и текущим определяет скорость полета огонька,
 * частота задаёт цвет.
 *
 * \author ARV
 * \date	15 дек. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 * \addtogroup FGEFFECT
 * \copybrief sparkler_effect.c См. \ref sparkler_effect.c
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

// Название эффекта, не более 16 знаков
static __flash const char _name[] = "SPARKLER";
// Описание эффекта. Можно не менять, можно для оптимизации заменить
// отсутствующие функции на NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.start	= _start,	// может быть NULL
	.work	= _effect,	// это значение не может быть NULL!
	.preset	= _preset	// может быть NULL
};

// количество одновременно летящих огней, не более
#define LIGHT_LIVE_CNT	10

typedef struct{
	uint8_t		pix;		//< номер пиксела
	uint8_t		color_id;	//< цвет огонька
	uint8_t		live;		//< время жизни
	uint8_t		speed;		//< скорость полёта
	uint8_t		div;		//< счетчик скорости
} light_t;

typedef struct{
	uint8_t		live; //< текущее количество огней
	uint8_t		h;
	light_t		lights[LIGHT_LIVE_CNT]; //< массив огней
	uint16_t	prev_peak[F_CNT]; //< массив предыдущего уровня
} static_parameter_t;

static static_parameter_t par;

static static_parameter_t *param=&par;

INIT(7){
	// регистрация эффекта - правильно указать тип эффекта!
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

// главная функция эффекта - вызывается 100 раз в секунду для рендеринга
static void _effect(signal_t *s){
	// прежде всего - обеспечиваем полет огней
	for(uint8_t i=0; i<LIGHT_LIVE_CNT; i++){
		// для каждого живого огонька
		if(param->lights[i].live){
			// если яркость упала до 0 - тоже убиваем огонь
			if(pixels[param->lights[i].pix].bright == 0){
				param->live--;
				continue;
			}
			//*/
			// если скорость позволяет
			if(!--param->lights[i].div){
				// обновляем делитель скорости
				param->lights[i].div = param->lights[i].speed;

				// уменьшаем время жизни
				param->lights[i].live--;

				if(++param->lights[i].pix >= PIXEL_CNT){
					// если улетели за пределы
					param->lights[i].live = 0;
				}
			}
			// если истекло - уменьшаем общий счетчик огней
			if(param->lights[i].live == 0) param->live--;
		}
	}
	if(1){
		// теперь анализ пиков и добавление новых огней
		int16_t delta;

		delta = s->harmonics[param->h] - param->prev_peak[param->h];
		if(delta > 50){
			param->prev_peak[param->h] = s->harmonics[param->h];
			// ищем свободный огонь в массиве
			uint8_t n;
			for(n=0; n<LIGHT_LIVE_CNT; n++){
				if(param->lights[n].live == 0){
					// нашли, его номер n
					param->lights[n].speed = get_speed(s->harmonics[param->h] / 10);
					param->lights[n].speed++; // чтобы не было 0
					param->lights[n].live = rand() % PIXEL_CNT;
					param->lights[n].color_id = param->h-1;
					param->lights[n].div = param->lights[n].speed;
					param->lights[n].pix = 0;
					if(param->lights[n].live) param->live++;
					break;
				}
			}
		}
		// уменьшаем порог
		if(param->prev_peak[param->h] >= 5) param->prev_peak[param->h] -= 5;

		param->h += 2;
		if(param->h >= F_CNT) param->h = 1;
		if(param->prev_peak[param->h] >= 5) param->prev_peak[param->h] >>= 1;
	}
	// теперь рисуем все огни

	off_all_pixels();
	for(uint8_t i=0; i<LIGHT_LIVE_CNT; i++){
		if(param->lights[i].live){
			set_fix_color(param->lights[i].pix, param->lights[i].color_id);
			bright_ctrl(param->lights[i].pix, 255, 10);
		}
	}
}

// начало эффекта - вызывается после того, как завершился другой эффект и
// в том случае, если функция _preset вернула значение, отличное от PRESET_NOTHING
static void _start(void){
	memset(param, 0, sizeof(static_parameter_t));
	param->h = 1;
}

// смена пресета эффекта - вызывается в произвольные моменты
static preset_result_t _preset(int8_t d){
	return PRESET_NOTHING;
}

/**
 * @}
 */
