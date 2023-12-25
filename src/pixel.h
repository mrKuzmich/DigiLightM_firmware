/** \file pixel.h
 * \brief ��������� ������ ��� ���������� ��������
 * ����� ��������������� ������� ��� ������� ����������� ��� ��������
 * RGB-��������
 * \author ARV
 * \date	6 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * \defgroup PIX ����������� ��� RGB-���������
 * \brief ������������ ���������� ����������� ������������
 *
 * ������ - ��� �������������� �������� ������ ��������� WS2812, �������� �
 * �������. ��� ������� ���������������, ��� ���� �������, ���� ��������� ��� �����
 * ������������� �����������. ��� ��������� �������� (��. \ref EFFECTS) ���������������,
 * ��� ��� ������� ����������� � ���� �������.
 * ���������� ��������� ������� ��������� ��� ����� � �������, ��� ���������
 * �������������� ������� �������� ��������.
 * \addtogroup PIX
 * @{
 */

#ifndef PIXEL_H_
#define PIXEL_H_
#include "color_transform.h"

/// ����� ���������� ��������
#define PIXEL_CNT		cfg.group_of_pixels/*32L*/
#define PIXEL_LEN		cfg.pixels_in_group
/// ������� ����� ��� ��������� �����, ����������� ��� ��������� ���� ��������
#define PIXEL_CNT_MASK	0x1F

/// ���������, ����������� ���� �������
typedef struct pixel_t{
	uint8_t		r;			///< ������� ������������ �����
	uint8_t		g;			///< ������� ������������ �����
	uint8_t		b;			///< ����� ������������ �����
	uint8_t		bright;		///< ������� �������
	int8_t		delta;		///< �������� �������� ��������������� ��������� �������
} pixel_t;

/// ���������� ���������������� ������
#define FIXED_COLORS		8
/// ����� ������ ����� ���������� #PIXEL_CNT_MASK
#define FIXED_COLORS_MASK	0x07

/// ����� ������� ����
#define RED		255, 0,	0
/// ����-������ ����
#define YELLOW	255, 255, 0
/// ��������� ����
#define ORANGE	239, 0x5C, 0
/// ����� ������� ����
#define GREEN	0, 255, 0
/// ������������� ����
#define LBLUE	0, 141, 210
/// ����� ����� ����
#define BLUE	0, 0, 255
/// ���������� ����
#define PURPLE	255, 0, 255
/// ������� ����������
#define COLOR1	0x3E, 4, 0x70
/// ����� ����
#define WHITE	255, 255, 255
/// ���������� ��������
#define BLACK	0, 0, 0

/// �������� �������� ��������������� ��������� ������� (��� ������, ��� ���������)
#define FADE_DIV	5
/// ������ ��������
extern pixel_t *pixels;

#define RND_PIX_DELTA	3

typedef enum rnd_pix_t {
	RND_PIX_NO_FADE = 0,
	RND_PIX_FADE_IN = -RND_PIX_DELTA,
	RND_PIX_FADE_OUT = RND_PIX_DELTA
} rnd_pix_t;

void init_pix_arr(uint8_t size);
/// �������������� ��������� ������� ���� ��������
void fade(void);
/// ��������� ������
void rnd_pix(uint8_t fix_cid, int8_t delta);
/// ������� ���������� ��������
uint8_t get_light_cnt(void);
/// ���������� ��������
void bright_ctrl(uint8_t id, uint8_t bright, int8_t delta);
/// ������ ������ �� ������������� ������
void set_fix_color(uint8_t pos, uint8_t fix_cid);
/// ������ ������������� �����
void set_rgb_color(uint8_t pos, uint8_t r, uint8_t g, uint8_t b);
void set_hsv_color(uint8_t pos, hsv_t hsv);
/// ���������� ���� ��������
void off_all_pixels(void);
void rnd_pix_rgb(uint8_t _r, uint8_t _g, uint8_t _b, int8_t delta);
/**
 * @}
 */
#endif /* PIXEL_H_ */
