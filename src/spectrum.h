/** \file spectrum.h
 * \brief Описания для модуля реализации спектрального анализа
 * \par
 * \author ARV
 * \n \date	2 апр. 2017 г.
 * \par
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \par
 * \addtogroup FFT
 * @{
 */

#ifndef SPECTRUM_H_
#define SPECTRUM_H_

#if defined(_FFT_DEBUG_)
extern int16_t real_offset;
#endif

/// Если есть желание убедиться, что двойная буфферизация дает выигрыш в производительности,
/// можно заремарить этот макрос и проверить, как будет БЕЗ неё.
#define DOUBLE_BUFFERED_SAMPLING

/// Захват семплов и обработка методом FFT
void sample_and_fft(void);
/// получение запрошенной гармоники FFT
uint16_t get_result_fft(uint8_t index);

/**
 * @}
 */
#endif /* SPECTRUM_H_ */
