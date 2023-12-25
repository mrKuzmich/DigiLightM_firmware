/** \file pixel.h
 * \brief сервисный модуль для реализации эффектов
 * Набор вспомогательных функций для базовых манипуляций над цепочкой
 * RGB-пикселов
 * \author ARV
 * \date	6 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * \defgroup PIX Манипуляции над RGB-пикселами
 * \brief Элементарное управление светящимися светодоидами
 *
 * Пиксел - это альтернативное название одного свтодиода WS2812, принятое в
 * проекте. Все пикселы рассматриваются, как одна линейка, хотя физически они могут
 * располагаться произвольно. При описаниях эффектов (см. \ref EFFECTS) подразумевается,
 * что все пикселы расположены в одну линейку.
 * Управление пикселами ведется раздельно для цвета и яркости, что позволяет
 * оптимизировать функции создания эффектов.
 * \addtogroup PIX
 * @{
 */

#ifndef PIXEL_H_
#define PIXEL_H_
#include "color_transform.h"

/// Общее количество пикселов
#define PIXEL_CNT		cfg.group_of_pixels/*32L*/
#define PIXEL_LEN		cfg.pixels_in_group
/// Битовая маска для выделения битов, достаточных для адресации всех пикселов
#define PIXEL_CNT_MASK	0x1F

/// Структура, описывающая один пиксель
typedef struct pixel_t{
	uint8_t		r;			///< красная составляющая цвета
	uint8_t		g;			///< зеленая составляющая цвета
	uint8_t		b;			///< синяя составляющая цвета
	uint8_t		bright;		///< яркость пиксела
	int8_t		delta;		///< условная скорость автоматического изменения яркости
} pixel_t;

/// Количество предопределенных цветов
#define FIXED_COLORS		8
/// Маска номера цвета аналогично #PIXEL_CNT_MASK
#define FIXED_COLORS_MASK	0x07

/// Чисто красный цвет
#define RED		255, 0,	0
/// Ярко-желтый цвет
#define YELLOW	255, 255, 0
/// Оранжевый цвет
#define ORANGE	239, 0x5C, 0
/// Чисто зеленый цвет
#define GREEN	0, 255, 0
/// Светлоголубой цвет
#define LBLUE	0, 141, 210
/// Чисто синий цвет
#define BLUE	0, 0, 255
/// фиолетовый цвет
#define PURPLE	255, 0, 255
/// Оттенок сиреневого
#define COLOR1	0x3E, 4, 0x70
/// Белый цвет
#define WHITE	255, 255, 255
/// Отсутствие свечения
#define BLACK	0, 0, 0

/// Условная скорость автоматического изменения яркости (чем больше, тем медленнее)
#define FADE_DIV	5
/// массив пикселей
extern pixel_t *pixels;

#define RND_PIX_DELTA	3

typedef enum rnd_pix_t {
	RND_PIX_NO_FADE = 0,
	RND_PIX_FADE_IN = -RND_PIX_DELTA,
	RND_PIX_FADE_OUT = RND_PIX_DELTA
} rnd_pix_t;

void init_pix_arr(uint8_t size);
/// Автоматическое изменение яркости всех пикселов
void fade(void);
/// Случайный пиксел
void rnd_pix(uint8_t fix_cid, int8_t delta);
/// Подсчет светящихся пикселов
uint8_t get_light_cnt(void);
/// Управление яркостью
void bright_ctrl(uint8_t id, uint8_t bright, int8_t delta);
/// Пиксел одного из фиксированных цветов
void set_fix_color(uint8_t pos, uint8_t fix_cid);
/// Пиксел произвольного цвета
void set_rgb_color(uint8_t pos, uint8_t r, uint8_t g, uint8_t b);
void set_hsv_color(uint8_t pos, hsv_t hsv);
/// Выключение всех пикселов
void off_all_pixels(void);
void rnd_pix_rgb(uint8_t _r, uint8_t _g, uint8_t _b, int8_t delta);
/**
 * @}
 */
#endif /* PIXEL_H_ */
