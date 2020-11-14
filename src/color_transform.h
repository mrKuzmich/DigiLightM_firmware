/** \file color_transform.h
 * \brief типы представления цвета разными способами
 * \author ARV
 * \date	5 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \defgroup COLOR Цветовая модель
 * \brief Модификации цветов
 *
 * Для WS2812 принята цветовая модель RGB, но для получения оттенков спектра,
 * особенно плавно меняющихся, удобнее применять моедель HSV.
 * \addtogroup COLOR
 * @{
 */

#ifndef COLOR_TRANSFORM_H_
#define COLOR_TRANSFORM_H_

#define HSV_GRADE	360

/// Тип для представления цвета в RGB-модели
typedef struct rgb_t {
	uint8_t		r;
	uint8_t		g;
	uint8_t		b;
} rgb_t;

/// Тип для представления цвета в HSV-модели
typedef struct hsv_t {
	uint16_t	h;	///< Оттенок цвета, значения в градусах 0...359
	uint8_t		s;	///< Насыщенность оттенка
	uint8_t		v;	///< Яркость оттенка
} hsv_t;

void rgb_to_hsv(rgb_t *src, hsv_t *dst);
void hsv_to_rgb(hsv_t *src, rgb_t *dst);

/**
 * @}
 */
#endif /* COLOR_TRANSFORM_H_ */
