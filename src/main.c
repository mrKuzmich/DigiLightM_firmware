/** \file main.c
 * \brief �������� ������ �������
 * \par
 * ������� ���� � ������� ���������� � ��������
 *
 * \author ARV
 * \date	4 ���. 2017 �.
 * \version 0.1	\par
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \mainpage �������� �����������
 * \par ��������
 * ������ "�������� �����������" � �������������� ����������� �����������
 * WS2812 � �������� ��������� ����������� ����������.
 * \par �������������
 * � ������� ������������:
 * - ���������� <a href="http://elm-chan.org/works/akilcd/report_e.html">FFT by Chan</a>
 * - ���������� <a href="http://elm-chan.org/fsw/ff/00index_p.html">Petit Fat FS by Chan</a>
 * - ��������� <a href="http://damian.pecke.tt/">DAMIAN PECKETT</a>
 * - ���� ������ � WS2812 ����� ��������� SPI �� <a href="http://forum.cxem.net/index.php?/profile/187660-mishany/">Mishany</a>
 * \par ����������:
 * -# avr-gcc 5.2.1 ��� ����� ����� ������
 * -# ����������� \b -Os
 * -# ���������� \b -flto
 * \par �������������� ������ �������:
 * - \ref COLOR
 * - \ref PIX
 * - \ref CONTROL
 * - \ref FFT
 * - \ref EFFECTS
 * - \ref PWR
 * - \ref WS
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <string.h>
#include <util/crc16.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "spectrum.h"
#include "ws2812.h"
#include "global.h"
#include "control.h"
#include "main_effect.h"
#include "pixel.h"
#include "power_control.h"

config_t cfg;

/** ������������� ���������� ��������������� ������������������
 * <b>KNOW-HOW � ARV</b> ��� ��������, ������������ ������� rand() ��� ������ �� �����������������
 * ��� ���������� ����� �������� ������� ��������������������: � ����� � ��� ��
 * ���������������� ����� ��������� ���������� ���� � �� �� ������������������
 * �����, � ������� ��� ��������� ����� ��������. ������ ������� ������ ��� ��������
 * ���������� ����������, ��������� �����������, ��� ����� ������ ������� ����������
 * ��� ���������������� �������� � ������������������ ������� ��������� "�����".
 */
static void  init_rand(void){
	uint8_t *ptr = (void*)RAMSTART;
	uint16_t seed;

	for(uint16_t i=0; i < RAMEND; i++){
		seed = _crc16_update(seed, *ptr++);
	}
	srand(seed);
}

#define DELTA_10MS	20000

static volatile uint8_t done_wait;

ISR(TIMER1_COMPA_vect){
	OCR1A = TCNT1 + DELTA_10MS;
	done_wait = 1;
}

/// ������������� ���������
INIT(7){
	init_rand();
	// 10 �� ������
	TIMSK |= _BV(OCIE1A);
	OCR1A = DELTA_10MS;
	TIFR = _BV(OCF1A);

#if defined(_DEBUG_)
	// ��������� ��������
	DDR(DEBUG_LED_PORT) |= _BV(DEBUG_LED_PIN0);
	debug_led_off();
#endif
}


/** �������������
 * ������� ������ ��� ������������� ������ � �������� �����. �� ����� � ���,
 * ��� ���� �������� ���� ������ ����� 10 ��, �� ��� ������ ��������, ��������
 * ������ ����� � 10 ��. ��� �����������, ��� ��� �������� � ������� �����
 * ����� ����������� � ������ �������������� 100 ��� � ������� (����� ���
 * �������� ��������).
 * ��� ������� ��� ��������� �������� CPU ������� �������� ���������
 * �� ����� ��������, �.�. ���� ��������� ��������, ��� ��������, ��� ��������
 * ���� �� ���������� ��������, � ���� �� �������� - ��� �����, �������� ������,
 * ��� �� 10 �� �� �������� ������������.
 */
static void synchronize(void){
	// ���� ���������� �� ��������� ������� ��������������� ������ 10 ��
	while(!done_wait){
		// ������, ���� ����
		debug_led_on();
	}
	debug_led_off();
	done_wait = 0;
}

// ������� ����������
#define AVE	64

typedef struct ave_t{
	uint8_t buf[AVE];
	uint8_t cur;
}ave_t;

/** ������ "���������� �������"
 *
 * @param ave ��������� �������
 * @param v ������� ������
 * @return �������� ������
 */
static uint8_t average(ave_t *ave, uint8_t v){
	uint16_t sum=0;

	ave->buf[ave->cur++] = v;
	if(ave->cur >= AVE) ave->cur = 0;

	for(uint8_t i=0; i<AVE; i++)
		sum += ave->buf[i];

	return sum / AVE;
}

// Single Pole Bandpass IIR Filter by DAMIAN PECKETT
static float bassFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2];
    xv[2] = sample / 9.1f;
    yv[0] = yv[1]; yv[1] = yv[2];
    yv[2] = (xv[2] - xv[0])
        + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

/** �������� �����
 * ��������� ������� ����� � �������
 * \note � �������� ��� ����� ����������� ����� ����� ����������� �������,
 * ������� �������� ������ ����� � ������, �� ����� ��������� � ��������.
 * @param s ��������� � ����������� �������
 */
static void beat_detect(signal_t *s){
	static uint8_t old=0;
	uint8_t trigger;
	// ��������� �����-�� �������
	int16_t new = bassFilter(s->harmonics[F_DC]);

	if(new > (300+s->average_vol)) trigger = 1;
	if(new < -10) trigger = 0;

	// ��������� ����� ������ � ������ ������������� �����-�� ��������
	s->beat = !old && trigger;
	old = trigger;
}

/// ��������� �������
signal_t	music;

static uint32_t sum_harmonics(signal_t *s, uint8_t from, uint8_t to){
	uint32_t result = 0;
	for(; from <= to; from++){
		result += s->harmonics[from];
	}
	return result;
}

static uint32_t get_band_power(signal_t *s, uint16_t band){
	uint32_t result = 0;
	uint8_t i = 0;

	for(uint16_t f=1; f; f <<= 1, i++){
		result += band & f ? s->harmonics[i] : 0;
	}
	return result;
}

static void prepare_samples(signal_t *s){
	static ave_t v_ave;

	// ��������� ���������
	for(uint8_t i=F_DC; i<F_CNT; i++)
		s->harmonics[i] = get_result_fft(i);

	// ��������� ���������
	//s->peak_volume = lock_input ? 0 : sum_harmonics(s, F_250_HZ, F_3800_HZ) / 6;
	s->peak_volume = get_band_power(s, BAND_FULL) / 6;
	s->average_vol = average(&v_ave, s->peak_volume);
	//s->average_vol = average(&v_ave, s->harmonics[F_DC]);

	// ��������� �������� �����
	s->band[LF] = sum_harmonics(s, F_250_HZ, F_500_HZ);
	s->band[MF] = sum_harmonics(s, F_750_HZ, F_1500_HZ);
	s->band[HF] = sum_harmonics(s, F_2800_HZ, F_3800_HZ) + s->harmonics[F_3800_HZ];
/*
	s->band[LF] = get_band_power(s, cfg.band_mask[LF]);
	s->band[MF] = get_band_power(s, cfg.band_mask[MF]);
	s->band[HF] = get_band_power(s, cfg.band_mask[HF]);
*/

	// ���������� �����
	beat_detect(&music);
}

// ����������� ������ ������� ������
uint8_t lock_input = 0;

MAIN(){
	uint8_t nosleep;

	while(1){
		power_off();
		nosleep = 1;
		reset_power_timeout();

		do{
			if(!lock_input){
				// ��������� ������� � FFT-����������
				sample_and_fft();			//  1 mS (7 mS without DOUBLE_BUFFERED_SAMPLING)
				// ���������� ���������� �������
				prepare_samples(&music);	// ~1 mS
			}
			// ���������
			fade();						// < 60 uS
			// ������������ ��������� �������
			execute_effect(&music);
			// ���������� ������� �����������
			ws2812_show();				// ~1.6 mS
			// ����������
			do_control(&music);			// ~2 uS
			// �������������� �������
			nosleep = !power_timeout(&music);
			// �������������
			synchronize();				//
		} while(nosleep);
	}
}

