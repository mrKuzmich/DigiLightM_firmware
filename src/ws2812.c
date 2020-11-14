/** \file ws2812.c
 * \brief ���������� ������������ WS2812 ��� ������ ����������� SPI
 *
 * \author ARV
 * \date	2 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \par
 * \defgroup WS ��������� WS2812 ��� ������ ����������� SPI
 * \brief ���������������� ����� ������ � WS2812
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

/// ��������� ��� ������������ �������� "0" ��� WS2812
#define WS_BIT_0	0xE0
/// ��������� ��� ������������ �������� "1" ��� WS2812
#define WS_BIT_1	0xF8

/** ��������� ������ 1 ����� ������ � WS2812
 *
 * @param byte dsdjlbvsq ,fqn
 */
__inline static void send_byte(uint8_t byte){
	for(uint8_t mask=0x80; mask; mask >>=1){
		SPDR = byte & mask ? WS_BIT_1 : WS_BIT_0;
		while(bit_is_clear(SPSR, SPIF));
	}
}

/** ���������� ����������� ������� ����������� WS2812
 *
 */
void ws2812_show(void){
	for(uint8_t i=0; i<PIXEL_CNT; i++){
		for(uint8_t j=0; j<PIXEL_LEN; j++){
			// ��� ���� WS2812 ������� �������� ������������ ������ ���� �����:
			send_byte((pixels[i].g * (pixels[i].bright+1))>>8);	// ������� �������
			send_byte((pixels[i].r * (pixels[i].bright+1))>>8);	// ����� �������
			send_byte((pixels[i].b * (pixels[i].bright+1))>>8);	// � ����� - �����
		}
	}
}

/** ������������� ����������� SPI
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
