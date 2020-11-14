/** \file color_transform.c
 * \brief Преобразования цветов разных моделей представления
 *
 * \author ARV
 * \n \date	5 апр. 2017 г.
 * \par
 * Copyright 2015 © ARV. All rights reserved.
 * \par
 * \note Функции надерганы из различных источников в интернете.
 * \addtogroup COLOR
 * @{
 */

#include <stdint.h>
#include "color_transform.h"

/* ******************************************************************************
 * Function rgb_to_hsv
 * Description: Converts an RGB color value into its equivalen in the HSV color space.
 * Copyright 2010 by George Ruinelli
 * The code I used as a source is from http://www.cs.rit.edu/~ncs/color/t_convert.html
 * Parameters:
 *   1. struct with RGB color (source)
 *   2. pointer to struct HSV color (target)
 * Notes:
 *   - r, g, b values are from 0..255
 *   - h = [0,360], s = [0,255], v = [0,255]
 *   - NB: if s == 0, then h = 0 (undefined)
 ******************************************************************************/
/** Преобразование RGB-цвета в HSV-цвет
 *
 * @param src исходный цвет
 * @param dst результирующее представление цвета
 */
void rgb_to_hsv(rgb_t *src, hsv_t *dst){
	uint8_t min, max, delta;

	if(src->r < src->g)
		min = src->r;
	else
		min = src->g;
	if(src->b < min)
		min = src->b;

	if(src->r > src->g)
		max = src->r;
	else
		max = src->g;
	if(src->b > max)
		max = src->b;

	dst->v = max;                // v, 0..255

	delta = max - min;                      // 0..255, < v

	if( max != 0 )
		dst->s = (uint16_t)(delta)*255 / max;        // s, 0..255
	else {
		// r = g = b = 0        // s = 0, v is undefined
		dst->s = 0;
		dst->h = 0;
		return;
	}

	if( src->r == max )
		dst->h = (src->g - src->b)*(HSV_GRADE/6)/delta;        // between yellow & magenta
	else if( src->g == max )
		dst->h = (HSV_GRADE/6)*2 + (src->b - src->r)*(HSV_GRADE/6)/delta;    // between cyan & yellow
	else
		dst->h = (HSV_GRADE/6)*4 + (src->r - src->g)*(HSV_GRADE/6)/delta;    // between magenta & cyan

	if( (int16_t)dst->h < 0 )
		dst->h += HSV_GRADE;
}

/** Преобразование HSV-цвета в RGB-цвет
 * \note Функция протестирована
 * @param src исходное представление цвета
 * @param dst результирующее представление цвета
 */
void hsv_to_rgb(hsv_t *src, rgb_t *dst){
	uint8_t hi,fr, p, q, t;
	uint8_t h_pr;

	src->h %= HSV_GRADE;

	if(src->s == 0) {
		/* color is grayscale */
		dst->r = dst->g = dst->b = src->v;
        return;
	}

	hi = src->h / (HSV_GRADE/6);

/*
	switch(hi) {
	case 0:
		h_pr = src->h;       break;
	case 1:
		h_pr = src->h - (HSV_GRADE/6);  break;
	case 2:
		h_pr = src->h - (HSV_GRADE/6)*2; break;
	case 3:
		h_pr = src->h - (HSV_GRADE/6)*3; break;
	case 4:
		h_pr = src->h - (HSV_GRADE/6)*4; break;
	case 5:
		h_pr = src->h - (HSV_GRADE/6)*5; break;
	}
*/
	h_pr = src->h - (HSV_GRADE/6) * hi;

	fr = ( h_pr * 255 ) / (HSV_GRADE/6);
	p  = src->v * ( 255 - src->s ) / 255;
	q  = src->v * ( 255 - ( ( src->s * fr ) / 255 ) ) / 255;
	t  = src->v * ( 255 - ( src->s * ( 255 - fr ) / 255 ) ) / 255;

	switch(hi) {
	case 0:
		dst->r = src->v;
		dst->g = t;
		dst->b = p;
		break;
	case 1:
		dst->r = q;
		dst->g = src->v;
		dst->b = p;
		break;
	case 2:
		dst->r = p;
		dst->g = src->v;
		dst->b = t;
		break;
	case 3:
		dst->r = p;
		dst->g = q;
		dst->b = src->v;
		break;
	case 4:
		dst->r = t;
		dst->g = p;
		dst->b = src->v;
		break;
	case 5:
		dst->r = src->v;
		dst->g = p;
		dst->b = q;
		break;
	}
}

/**
 * @}
 */
