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

static mgain_t _mgain;

/**
 * ��������� �������� ������ ���������
 * GAIN_PORT.MC_GAIN_PIN -> z-60dB; 0-50dB; 1-40dB
 */
void _set_mgain(void) {
    switch (_mgain) {
        case MG_40DB:
            // ��� ���������� �� �����, �������� 1
            DDR(GAIN_PORT) |= MC_GAIN_PIN;
            PORT(GAIN_PORT) |= MC_GAIN_PIN;
            break;
        case MG_50DB:
            // ��� ���������� �� �����, �������� 0
            DDR(GAIN_PORT) |= MC_GAIN_PIN;
            PORT(GAIN_PORT) &= ~MC_GAIN_PIN;
            break;
        default: // ����� �������� 60dB
            // ��� ���������� �� ����, �������� 0
            DDR(GAIN_PORT) &= ~MC_GAIN_PIN;
            PORT(GAIN_PORT) &= ~MC_GAIN_PIN;
    }
}

/**
 * ������������� ����� ���������� ���������
 */
INIT(7){
	// TIMER2 ������������ ��� ���-������� ���������� ���������
	TCCR2 = TIMER_CLK_DIV_1 | _BV(WGM20) | _BV(WGM21) | _BV(COM21);
	// ����� ��� �� 2-�� �������
	DDR(GAIN_PORT) |= GAIN_PIN; // ����� OC2

	_mgain = cfg.sensitivity[IN_MIC];
	_set_mgain();
}

/**
 * ���������� ������������� ���������������� ����� ��������� ���������� ���
 * @param v ����������� ���������������
 */
static void set_gain(uint8_t v){
	OCR2 = (MAX_LEVEL - v);
}

void change_mgain(void) {
    if (_mgain != cfg.sensitivity[IN_MIC]) {
        _mgain = cfg.sensitivity[IN_MIC];
        _set_mgain();
    }
};

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
			integral = cfg.sensitivity[cfg.input] / 0.008;
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
		cfg.sensitivity[cfg.input] = v;
	}

	set_gain(cfg.sensitivity[cfg.input]);
}

/**
 * @}
 */
