/** \file spectrum.h
 * \brief �������� ��� ������ ���������� ������������� �������
 * \par
 * \author ARV
 * \n \date	2 ���. 2017 �.
 * \par
 * \copyright Copyright 2015 � ARV. All rights reserved.
 * \par
 * \addtogroup FFT
 * @{
 */

#ifndef SPECTRUM_H_
#define SPECTRUM_H_

#if defined(_FFT_DEBUG_)
extern int16_t real_offset;
#endif

/// ���� ���� ������� ���������, ��� ������� ������������ ���� ������� � ������������������,
/// ����� ���������� ���� ������ � ���������, ��� ����� ��� ��.
#define DOUBLE_BUFFERED_SAMPLING

/// ������ ������� � ��������� ������� FFT
void sample_and_fft(void);
/// ��������� ����������� ��������� FFT
uint16_t get_result_fft(uint8_t index);

/**
 * @}
 */
#endif /* SPECTRUM_H_ */
