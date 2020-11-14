/** \file spectrum.c
 * \brief ������������ ������
 * \par
 * \author ARV
 * \n \date	11.04.2017
 * \par
 * \copyright Copyright 2008 � ARV. All rights reserved.
 *
 * \defgroup FFT ������������ ������ �����
 * \brief ��������� ��������� ������� ������� FFT
 * \par
 * ������ ���������� � ����������� FFT �� ��������� ����� Chen-� - ������������
 * ���������� ������ ����������� FFT ��� ����������������� AVR.
 * \par
 * � ������� ���������� ������ ������� ������� ������������ ��� FFT, ���
 * ����������� ����������� ����� ������������������ - ��. #DOUBLE_BUFFERED_SAMPLING
 * ������� ������������ ����������� � ���, ��� ���� � ���� ������ �� �����������
 * ��������� ������ �������, ������ ������ ������������ FFT-��������������, �
 * ����� ������� �������� �������.
 * \addtogroup FFT
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "hardware.h"
#include "fft32.h"
#include "spectrum.h"
#include "global.h"

#if defined(DOUBLE_BUFFERED_SAMPLING)
#define FFT_BUFFER_SIZE	(FFT_N * 2L)
#else
#define FFT_BUFFER_SIZE	FFT_N
#endif

/// ������ ��� ��� �1
static complex_t buffer_fft_a[FFT_BUFFER_SIZE];
#if defined(DOUBLE_BUFFERED_SAMPLING)
/// ������ ��� ��� �2
static complex_t *buffer_fft_b = buffer_fft_a + FFT_N;//[FFT_N];
/// ������� ��������� �� ������� �������� �����
static complex_t *buf_out = buffer_fft_a + FFT_N;//buffer_fft_b;
#endif
/// ������� ��������� �� ������� ������� �����
static complex_t *buf_in = buffer_fft_a;

/**
 * ������� ���������� ��������� �� ���� ��������� ������, ��������� ����� �������
 * @param size � ���� ��������� ������� �������� ������ ������������� ������, �
 * � ��� �� ���������� ���������� ���������� ���������� ������.
 * @return ��������� �� ���� ��������� ������, NULL ���� ��������� ������ ���
 */
void* get_reserved_memory(uint16_t *size){
	if(!lock_input) return NULL;
	if(*size > (FFT_BUFFER_SIZE*sizeof(complex_t)))
		*size = (FFT_BUFFER_SIZE*sizeof(complex_t));
	return buffer_fft_a;
}

/// ��������� �� "�������" ������
static __flash const int16_t *window_ptr = tbl32_window;
/// ������� ������� ��� ������ ������, ������������ � ���������� ���
static volatile uint8_t adc_count;

#if defined(_FFT_DEBUG_)
int16_t real_offset;
#endif

/**
 * ���������� ���������� �� ���������� ���.
 * ��������� ����� ���������� ������� �������� �������
 */
ISR(ADC_vect){
#if defined(_FFT_DEBUG_)
	real_offset = ADC;
#endif
	int16_t v = ADC - cfg.dc_offset;

	v = fmuls_f(v, *window_ptr);

	buf_in->r = v;				// ������� ��� � ������ ���
	buf_in->i = v;

	window_ptr++;
	buf_in++;					// �������� ��������� �� ��������� �������
	// �.�. ������ ��� �� ������� �������� ������ ���� ���� ���������������� �������
	// ������� ������������� �� ����������� �������, ���������� ���� ����������� �������
	TIFR = _BV(OCF0);
	// �������� �� ��� ���, ���� �� ������� �������� ���������� ������
	if(!--adc_count){
		TCCR0 = 0;				// ���������� ������, ����� ����� ��������
	}
}

/**
 * �������������� ���:
 * ���������� AREF
 * ������������ ���������� �����
 * ���������� �� ������� - ������������ ����������� 0-�� �������
 */
INIT(7){
	DDR(ADC_PORT) &= ~_BV(ADC_PIN);

    SFIOR = _BV(ADTS1) | _BV(ADTS0);

	ADCSRA	= _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | ADC_DIV_32;
	//ADMUX	= _BV(ADLAR) | ADC_REF_INT_WITH_ECAP;	// ���������� AREF
	ADMUX = _BV(ADLAR) | ADC_REF_AVCC; // ����������� ����� "���������" ��������
	OCR0 = 250; //!!! ��� ������� ������� 8000 ��, �.�. ������ ������� 3750 ��

	sei();
}

/**
 * ������ ������� �������
 * � ������� �������������
 * ��� ��� ��� \see DOUBLE_BUFFERED_SAMPLING
 */
void sample_and_fft(void){
	static uint8_t buf_id;

#if defined(DOUBLE_BUFFERED_SAMPLING)

	while(TCCR0);					// ����, ���� �� �� �����������

	// ������ ������ �������
	if(buf_id){
		buf_in = buffer_fft_b;
		buf_out = buffer_fft_a;
	} else {
		buf_in = buffer_fft_a;
		buf_out = buffer_fft_b;
	}
#else
	buf_in = buffer_fft_a;
#endif

	buf_id = !buf_id;

	adc_count = FFT_N;
	window_ptr = tbl32_window;
	TCNT0 = 0;

	TCCR0 = _BV(WGM01) | TIMER_CLK_DIV_8;	// ��������� 0-� ������

#if !defined(DOUBLE_BUFFERED_SAMPLING)
	while(TCCR0);					// ����, ���� �� �� �����������
	// ����������� ������ ��� ���������� �����, ���� ������ ����������� ��������
	fft32_execute(buffer_fft_a);		// ��������� ��� ��� �������
#else
	// ����������� ������ ��� ���������� �����, ���� ������ ����������� ��������
	fft32_execute(buf_out);		// ��������� ��� ��� �������
#endif
}

/**
 * ���������� �������� ��������� ��������� �� ����������� ���
 * @param index ���������� ����� ���������
 * @return ������������� ������� ���������
 */
uint16_t get_result_fft(uint8_t index){
	uint16_t ix;
	__flash const int16_t *revers = tbl32_bitrev;

	revers += index;				// ��������� ��������� �� ������ ������������
	ix = *revers;		// ��������� ������
	return (
#if defined(DOUBLE_BUFFERED_SAMPLING)
	// ���������� �� ������� ��� �������� �� ������� ������������
	fsqrt(buf_out[ix].r,buf_out[ix].i)
#else
	fsqrt(buffer_fft_a[ix].r,buffer_fft_a[ix].i)
#endif
	// �� ����� ���� ������� (������) ���� ������������� � ��������� �����������
	// ����� ������
	 & 0xFFC0)>> 1;
}

/**
 * @}
 */
