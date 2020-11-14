/** \file pixel.c
 * \brief Вспомогательные функции манипуляции пикселами
 * \par
 *
 * \author ARV
 * \n \date	6 апр. 2017 г.
 * \par
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \par
 *
 * \addtogroup PIX
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <stdlib.h>
#include "global.h"
#include "pixel.h"

/// Массив пикселей предельного размера
pixel_t	pixels[MAX_TOTAL_PIX];

/// Массив предопределенных цветов
static const
__flash rgb_t fix_colors[FIXED_COLORS] = {
	{RED}, {YELLOW}, {GREEN}, {LBLUE}, {ORANGE}, {BLUE}, {PURPLE}, {COLOR1}
};

static void fade_out(pixel_t *px){
	if(px->delta < 0){
		if((px->bright - px->delta) > 255){
			px->bright = 255;
			px->delta = -px->delta;
		}else
			px->bright -= px->delta;
	} else {
		if(px->bright >= px->delta)
			px->bright -= px->delta;
		else
			px->bright = 0;
	}
}

/**
 * При каждом вызове этой функции яркость каждого пиксела изменяется на величину
 * #pixel_t.delta - см. #fade.
 */
void fade(void){
	static uint8_t div = FADE_DIV;
	if(--div) return;
	div = FADE_DIV;
	pixel_t *pixel = pixels;
	for(uint8_t i=0; i<PIXEL_CNT; i++){
		fade_out(pixel++);
	}
}

/**
 * Так как цвет и яркость пиксела независимы, для определения того, что пиксел
 * светится, недостаточно просто проверить его яркость (например, пиксел с
 * цветом [R:1,G:1,B:1] и яркостью 1 на самом деле светиться не будет).
 * @param id номер проверемого пиксела (менее #PIXEL_CNT)
 * @return 1, если пиксел светится, иначе - 0
 */
static uint8_t is_light_pix(uint8_t id){
	return !!((uint16_t)(
			(pixels[id].r * (pixels[id].bright+1)) |
			(pixels[id].g * (pixels[id].bright+1)) |
			(pixels[id].b * (pixels[id].bright+1)))>>8
	);
}

/**
 * Функция просто подсчитывает количество светящихся пикселей
 * @return количество светящихся пикселей
 */
uint8_t get_light_cnt(void){
	uint8_t sum = 0;
	for(uint8_t i=0; i<PIXEL_CNT; i++)
		sum += is_light_pix(i);
	return sum;
}

/**
 * Управление параметрами яркости пиксела
 * @param id номер проверемого пиксела (менее #PIXEL_CNT)
 * @param bright задаваемый уровень яркости
 * @param delta задаваемый уровень скорости изменения яркости
 * \note Если delta меньше нуля, яркость автоматически будет возрастать до максимума,
 * а затем начнет с тем же темпом уменьшаться до нуля.
 * При положительном значении delta яркость уменьшается. Если delta равно 0, то
 * яркость пиксела с течением времени не меняется
 */
void bright_ctrl(uint8_t id, uint8_t bright, int8_t delta){
	pixels[id].bright = bright;
	pixels[id].delta = delta;
}

/**
 * Цвет пиксела в формате RGB
 * @param pos номер проверемого пиксела (менее #PIXEL_CNT)
 * @param r уровень красной составляющей цвета
 * @param g уровень зеленой составляющей цвета
 * @param b уровень синей составляющей цвета
 */
void set_rgb_color(uint8_t pos, uint8_t r, uint8_t g, uint8_t b){
	pixels[pos].r = r;
	pixels[pos].g = g;
	pixels[pos].b = b;
}

void set_hsv_color(uint8_t pos, hsv_t hsv){
	rgb_t rgb;
	hsv_to_rgb(&hsv, &rgb);
	set_rgb_color(pos, rgb.r, rgb.g, rgb.b);
}

/**
 * Задание цвета пиксела из числа предопределенных цветов
 * @param pos номер проверемого пиксела (менее #PIXEL_CNT)
 * @param fix_cid номер устанавливаемого цвета (см. #fix_colors)
 */
void set_fix_color(uint8_t pos, uint8_t fix_cid){
	pixels[pos].r = fix_colors[fix_cid].r;
	pixels[pos].g = fix_colors[fix_cid].g;
	pixels[pos].b = fix_colors[fix_cid].b;
}

/**
 * Включает пиксел в случайной позиции в указанный цвет и с указанной скоростью
 * затухания
 * @param fix_cid номер устанавливаемого цвета (см. #fix_colors)
 * @param delta задаваемая скорость автоматического изменения яркости
 * \note Фунция зажигает пиксел только на свободном месте, т.е. если уже светится
 * много пикселов, функция ищет свободное место, что требует времени.
 * Так как автоматическое изменение яркости реализуется вне этой функции, то
 * нельзя вызывать ее в моменты, когда <b>все пикселы светятся</b> -
 * она никогда не завершится!
 */
void rnd_pix(uint8_t fix_cid, int8_t delta){
	uint8_t id;

	//do id = (rand() >> 4) & PIXEL_CNT_MASK; while((id >= PIXEL_CNT) || is_light_pix(id));

	do id = rand() % PIXEL_CNT; while (is_light_pix(id));

	bright_ctrl(id, delta >= 0 ? 255 : 0, delta);
	set_fix_color(id, fix_cid);
}

void rnd_pix_rgb(uint8_t _r, uint8_t _g, uint8_t _b, int8_t delta){
	uint8_t id;

	//do id = (rand() >> 4) & PIXEL_CNT_MASK; while((id >= PIXEL_CNT) || is_light_pix(id));
	do id = rand() % PIXEL_CNT; while (is_light_pix(id));

	bright_ctrl(id, delta >= 0 ? 255 : 0, delta);
	set_rgb_color(id, _r, _g, _b);
}

void off_all_pixels(void){
	for(uint8_t i=0; i < PIXEL_CNT; i++)
		bright_ctrl(i, 0, 0);
}
/**
 * @}
 */
