/** \file sd_script.h
 * \brief Описания модуля переключателя входов
 *
 *
 * \author DVK
 * \date	19.05.2020
 * \copyright 
 *
 * Для компиляции требуется:
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#ifndef _IN_SWITCH_H_
#define _IN_SWITCH_H_

typedef enum {
	IN_OFF, IN_1, IN_2, IN_MIC,
	_IN_CNT
} input_t;

#define IN_CNT		_IN_CNT

void change_input(void);

#endif

