/** \file gain_ctrl.c
 * \brief Управление усилением
 *
 * Модуль, реализующий возможность автоматического или ручного регулирования
 * чувствительности входного сигнала.
 *
 * \author ARV
 * \date	15 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
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
 * Инициализация линий управления усилением
 */
INIT(7){
	// TIMER2 используется для ШИМ-сигнала управления усилением
	TCCR2 = TIMER_CLK_DIV_1 | _BV(WGM20) | _BV(WGM21) | _BV(COM21);
	// выход ШИМ от 2-го таймера
	DDR(OC2_PORT) |= OC2_PIN; // выход OC2
}

/**
 * Управление коэффициентом чувствительности путем изменения скважности ШИМ
 * @param v коэффициент масштабирования
 */
static void set_gain(uint8_t v){
	OCR2 = (MAX_LEVEL - v);
}

/**
 * Стремится удерживать средний уровень сигнала вблизи заданного уровня #AGC_NORMAL_LVL
 * плюс-минус величина #AGC_VARIATION.
 * @param s параметры звукового сигнала.
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
