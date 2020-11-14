/** \file ws2812.c
 * \brief Управление светодиодами WS2812 при помощи аппаратного SPI
 *
 * \author ARV
 * \date	2 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \par
 * \defgroup WS Поддержка WS2812 при помощи аппаратного SPI
 * \brief Высокоскоростной вывод данных в WS2812
 *
 * \addtogroup WS
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <util/delay.h>
#include "global.h"
#include "pixel.h"
#include "ws2812.h"

#if (F_CPU < 16000000UL) || !defined(F_CPU)
#error "F_CPU must be defined and must be greather or equal 16MHz"
#endif

/// Константа для формирования импульса "0" для WS2812
#define WS_BIT_0	0xE0
/// Константа для формирования импульса "1" для WS2812
#define WS_BIT_1	0xF8

/** Побитовая выдача 1 байта данных в WS2812
 *
 * @param byte dsdjlbvsq ,fqn
 */
__inline static void send_byte(uint8_t byte){
	for(uint8_t mask=0x80; mask; mask >>=1){
		SPDR = byte & mask ? WS_BIT_1 : WS_BIT_0;
		while(bit_is_clear(SPSR, SPIF));
	}
}

/** Обновление содержимого цепочки светодиодов WS2812
 *
 */
void ws2812_show(void){
	for(uint8_t i=0; i<PIXEL_CNT; i++){
		for(uint8_t j=0; j<PIXEL_LEN; j++){
			// для чипа WS2812 порядок цветовых составляющих должен быть таким:
			send_byte((pixels[i].g * (pixels[i].bright+1))>>8);	// сначала ЗЕЛЕНЫЙ
			send_byte((pixels[i].r * (pixels[i].bright+1))>>8);	// затем КРАСНЫЙ
			send_byte((pixels[i].b * (pixels[i].bright+1))>>8);	// в конце - СИНИЙ
		}
	}
}

/** Инициализация аппаратного SPI
 *
 */
INIT(7){
	DDR(PORT_SPI) |= _BV(PIN_MOSI) | _BV(PIN_SS) | _BV(PIN_SCK);

	SPCR = _BV(SPE) | _BV(MSTR) | _BV(CPHA);//fosc/2 16MHz/2
	SPSR = _BV(SPI2X);
	SPSR; SPDR;
}

/**
 * @}
 */
