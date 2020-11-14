/** \file gain_ctrl.c
 * \brief ���������� ���������
 *
 * ������, ����������� ����������� ��������������� ��� ������� �������������
 * ���������������� �������� �������.
 *
 * \author ARV
 * \date	15 ���. 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \addtogroup CONTROL
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <stdlib.h>
#include "hardware.h"
#include "global.h"
#include "gain_ctrl.h"
#include "spectrum.h"

/**
 * ������������� ����� ���������� ���������
 */
INIT(7){
	// TIMER2 ������������ ��� ���-������� ���������� ���������
	TCCR2 = TIMER_CLK_DIV_1 | _BV(WGM20) | _BV(WGM21) | _BV(COM21);
	// ����� ��� �� 2-�� �������
	DDR(OC2_PORT) |= OC2_PIN; // ����� OC2
}

/**
 * ���������� ������������� ���������������� ����� ��������� ���������� ���
 * @param v ����������� ���������������
 */
static void set_gain(uint8_t v){
	OCR2 = (MAX_LEVEL - v);
}

/**
 * ��������� ���������� ������� ������� ������� ������ ��������� ������ #AGC_NORMAL_LVL
 * ����-����� �������� #AGC_VARIATION.
 * @param s ��������� ��������� �������.
 */
void sens_control(signal_t *s){
	static uint8_t div = GAIN_DIV;
	static float integral = 0.0;

	if(cfg.agc_enabled && (s != NULL)){
		if(s->average_vol < DEF_SILENCE_LVL) {
			integral = cfg.sensitivity / 0.008;
			return;
		}
		// automatic - PID
		if(--div) return;
		div = GAIN_DIV;

		float error = (AGC_NORMAL_LVL - s->peak_volume);
		if(error < 0)
			integral += error;
		else
			integral += 0.1*error;
		if(integral < 0) integral = 0;
		if(integral > 10000.0) integral = 10000.0;

		float v = error * 0.05 + integral * 0.008;
		if(v > (MAX_LEVEL-1)) v = MAX_LEVEL - 1;
		if(v < 0) v = 0;
		cfg.sensitivity = v;
	}

	set_gain(cfg.sensitivity);
}

/**
 * @}
 */
