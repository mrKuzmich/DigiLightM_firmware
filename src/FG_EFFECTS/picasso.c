/** \file picasso.c
 * \brief \b "PICASSO"
 * \n Ёффект на основе обработки разности уровней полос.
 *
 * \author ARV
 * \date	19 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \addtogroup FGEFFECTS
 * \copybrief picasso.c —м. \ref picasso.c
 *
 */

#include <avr/io.h>
#include "../avr_helper.h"
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include "../color_transform.h"
#include "../global.h"
#include "../pixel.h"
#include "../main_effect.h"
/**
 * @{
 */
static void _effect(signal_t *s);
static void _stop(void);
static void _start(void);
static preset_result_t _preset(int8_t d);
static void _save(void);

// Ќазвание эффекта, не более 16 знаков
static __flash const char _name[] = "PICASSO";
// ќписание эффекта. ћожно не мен€ть, можно дл€ оптимизации заменить
// отсутствующие функции на NULL
static flash_effect_t effect_def = {
	.name	= _name,
	.work	= _effect	// это значение не может быть NULL!
};

// ќ“ Ћё„≈Ќќ ƒќ ¬џя—Ќ≈Ќ»я
INIT(7){
	// регистраци€ эффекта - правильно указать тип эффекта!
	//register_effect(FOREGROUND_EFFECT, &effect_def);
}

#define PIC_SZ			4
#define DELTA_LIMIT		500
#define HARM			4

// массив номеров полос анализа
static __flash const uint8_t harm[HARM] = {
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
		3, 7, 10, 15
};

static uint8_t DIV = 1;
static int8_t offset;

typedef struct{
	uint8_t predelay;	///< задержка перед началом погасани€
	uint16_t peak;		///< предыдущий пиковый уровень
	uint8_t picture[PIC_SZ]; ///< "рисунок"
} delta_t;

static delta_t delta[HARM];

uint16_t rand_range(uint16_t range){
	return rand() % range;
}

uint16_t rand_deviation(uint16_t center, uint16_t dev_width){
	return center + dev_width / 2 - rand_range(dev_width);
}

static void local_fade(uint8_t f){
	for(uint8_t j=0; j<PIC_SZ; j++){
		// ждем, пока истечет врем€ ожидани€
		if(delta[f].predelay)
			delta[f].predelay--;
		else {
			// после чего быстро уменьшаем €ркость
			// TODO спад €ркости после выжидани€
			if(pixels[delta[f].picture[j]].delta == 1){
				pixels[delta[f].picture[j]].bright /= 2;
				pixels[delta[f].picture[j]].delta = 50;
			}
			// и порог
			if(delta[f].peak >= 0) delta[f].peak -= 50;
		}
	}
}

static void make_picture(uint8_t f, uint16_t peak){
	uint8_t k = PIC_SZ;
	hsv_t color = {330,255,255};

	// очищаем рисунок
	memset(delta[f].picture, 0, PIC_SZ);

	while(peak && k){
		// выбираем случайный пиксел, т€готеющий к пропорциональной частоте позиции на линейке
		delta[f].picture[k-1] = rand_deviation(f*(PIXEL_CNT/HARM), 10) % PIXEL_CNT;
		// сразу ему задаем €ркость и дельту
		pixels[delta[f].picture[k-1]].bright = 255;
		pixels[delta[f].picture[k-1]].delta = 1;
		// и назначаем цвет пропорционально частоте
		color.h = (HSV_GRADE / HARM) * f;
		set_hsv_color(delta[f].picture[k-1], color);
		k--;
		peak /= 2;
	}
}

static void paint2(signal_t *s){
	static uint8_t wait[32]; // TODO вылетает за пределы массива, нужен динамический маччив

	for(uint8_t i=0; i<PIXEL_CNT; i++){
		if(wait[i])
			wait[i]--;
		else {
			pixels[i].delta = 25;
		}
	}
	uint16_t peak = 0;
	uint16_t p1;
	uint8_t  peak_f;

	// перебираем все частоты
	for(uint8_t f=1; f < F_CNT; f++){
		// выбираем наибольшую амплитуду
		p1 = (s->harmonics[f] / (1+3/f));
		if(p1 > peak){
			peak = p1;
			peak_f = f;
		}
	}
	// только дл€ частоты с наибольшей амплитудой
	if((peak >> 2) > 80) {
		wait[peak_f] = 40;
		hsv_t color = {330,255,255};
		color.h = offset+(HSV_GRADE / HARM) * (peak_f-1);
		uint8_t pix = rand_range(PIXEL_CNT);
		set_hsv_color(pix, color);
		bright_ctrl(pix, 255, 1);
	}
}

// главна€ функци€ эффекта - вызываетс€ 100 раз в секунду дл€ рендеринга
static void _effect(signal_t *s){
	static uint8_t	div = 1;


	if(--div) return;
	div = DIV;

	paint2(s);
}


/**
 * }@
 */
