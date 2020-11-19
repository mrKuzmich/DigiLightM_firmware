/** \file ws2812.h
 * \brief аппаратные особенности подключения WS2812
 * \par
 *
 * \author ARV
 * \n \date	2 апр. 2017 г.
 * \par
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \par
 *
 * \addtogroup WS
 * @{
 */

#ifndef WS2812_H_
#define WS2812_H_

#include "hardware.h"

// Base colors codes
#define C_R   0x01
#define C_G   0x02
#define C_B   0x03

// Colors sequences
#define S_RGB (C_B<<4)|(C_G<<2)|(C_R)
#define S_RBG (C_G<<4)|(C_B<<2)|(C_R)
#define S_GRB (C_B<<4)|(C_R<<2)|(C_G)
#define S_GBR (C_R<<4)|(C_B<<2)|(C_G)
#define S_BRG (C_G<<4)|(C_R<<2)|(C_B)
#define S_BGR (C_R<<4)|(C_G<<2)|(C_B)
#define COLOR_SEQ_COUNT 6
#define COLOR_MASK  0b00000011

static uint8_t color_sequence[] = {S_RGB, S_RBG, S_BRG, S_BGR, S_GBR, S_GRB};

/// обновление ленты светододов
void ws2812_show(void);

/**
 * @}
 */
#endif /* WS2812_H_ */
