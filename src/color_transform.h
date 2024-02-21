/** \file color_transform.h
 * \brief ���� ������������� ����� ������� ���������
 * \author ARV
 * \date	5 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \defgroup COLOR �������� ������
 * \brief ����������� ������
 *
 * ��� WS2812 ������� �������� ������ RGB, �� ��� ��������� �������� �������,
 * �������� ������ ����������, ������� ��������� ������ HSV.
 * \addtogroup COLOR
 * @{
 */

#ifndef COLOR_TRANSFORM_H_
#define COLOR_TRANSFORM_H_

#define HSV_GRADE	360

#if (HSV_GRADE > 256)
typedef uint16_t h_type;
#else
typedef uint8_t h_type;
#endif

/// ��� ��� ������������� ����� � RGB-������
typedef struct rgb_t {
	uint8_t		r;
	uint8_t		g;
	uint8_t		b;
} rgb_t;

/// ��� ��� ������������� ����� � HSV-������
typedef struct hsv_t {
	uint16_t	h;	///< ������� �����, �������� � �������� 0...359
	uint8_t		s;	///< ������������ �������
	uint8_t		v;	///< ������� �������
} hsv_t;

void rgb_to_hsv(rgb_t *src, hsv_t *dst);
void hsv_to_rgb(hsv_t *src, rgb_t *dst);
void rgb_from_hue(h_type h, rgb_t *dst);

/**
 * @}
 */
#endif /* COLOR_TRANSFORM_H_ */
