/** \file fft_debug.h
 * \brief отладочный модуль
 * \par
 * Включает индикацию на ЖКИ внутренних параметров прошивки
 * \author ARV
 * \date	6 июня 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#ifndef FFT_DEBUG_H_
#define FFT_DEBUG_H_

void _fft_debug_show(signal_t *s);

#endif /* FFT_DEBUG_H_ */
