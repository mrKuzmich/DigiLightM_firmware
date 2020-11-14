/** \file spectrum.c
 * \brief спектральный анализ
 * \par
 * \author ARV
 * \n \date	11.04.2017
 * \par
 * \copyright Copyright 2008 © ARV. All rights reserved.
 *
 * \defgroup FFT Спектральный анализ звука
 * \brief Обработка звукового сигнала методом FFT
 * \par
 * Модуль сопряжения с библиотекой FFT от японского гения Chen-а - единственной
 * реализации быстро работающего FFT для микроконтроллеров AVR.
 * \par
 * В текущей реализации модуля введена двойная буфферизация для FFT, что
 * кардинально увеличивает общую производительность - см. #DOUBLE_BUFFERED_SAMPLING
 * Двойная буфферизация заключается в том, что пока в один массив по прерываниям
 * заносятся семплы сигнала, другой массив подвергается FFT-преобразованию, а
 * затем массивы меняются местами.
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

/// массив для БПФ №1
static complex_t buffer_fft_a[FFT_BUFFER_SIZE];
#if defined(DOUBLE_BUFFERED_SAMPLING)
/// массив для БПФ №2
static complex_t *buffer_fft_b = buffer_fft_a + FFT_N;//[FFT_N];
/// рабочий указатель на текущий ВЫХОДНОЙ буфер
static complex_t *buf_out = buffer_fft_a + FFT_N;//buffer_fft_b;
#endif
/// рабочий указатель на текущий ВХОДНОЙ буфер
static complex_t *buf_in = buffer_fft_a;

/**
 * Функция возвращает указатель на блок резервной памяти, доступный любым модулям
 * @param size в этом параметре функция получает размер запрашиваемой памяти, и
 * в нем же возвращает количество фактически выделенной памяти.
 * @return указатель на блок резервной памяти, NULL если доступной памяти нет
 */
void* get_reserved_memory(uint16_t *size){
	if(!lock_input) return NULL;
	if(*size > (FFT_BUFFER_SIZE*sizeof(complex_t)))
		*size = (FFT_BUFFER_SIZE*sizeof(complex_t));
	return buffer_fft_a;
}

/// указатель на "оконный" массив
static __flash const int16_t *window_ptr = tbl32_window;
/// счетчик семплов для сборки данных, используется в прерывании АЦП
static volatile uint8_t adc_count;

#if defined(_FFT_DEBUG_)
int16_t real_offset;
#endif

/**
 * Обработчик прерывания по завершению АЦП.
 * Заполняет буфер значениями семплов входного сигнала
 */
ISR(ADC_vect){
#if defined(_FFT_DEBUG_)
	real_offset = ADC;
#endif
	int16_t v = ADC - cfg.dc_offset;

	v = fmuls_f(v, *window_ptr);

	buf_in->r = v;				// заносим его в массив БПФ
	buf_in->i = v;

	window_ptr++;
	buf_in++;					// сдвигаем указатель на следующий элемент
	// т.к. запуск АЦП по событию возможен только если флаг соответствующего события
	// сброшен принудительно ДО НАСТУПЛЕНИЯ СОБЫТИЯ, сбрасываем флаг компаратора таймера
	TIFR = _BV(OCF0);
	// работаем до тех пор, пока не соберем заданное количество семлов
	if(!--adc_count){
		TCCR0 = 0;				// остановить таймер, когда буфер заполнен
	}
}

/**
 * инициализирует АЦП:
 * встроенный AREF
 * выравнивание результата влево
 * автозапуск по событию - срабатыванию компаратроа 0-го таймера
 */
INIT(7){
	DDR(ADC_PORT) &= ~_BV(ADC_PIN);

    SFIOR = _BV(ADTS1) | _BV(ADTS0);

	ADCSRA	= _BV(ADEN) | _BV(ADIE) | _BV(ADATE) | ADC_DIV_32;
	//ADMUX	= _BV(ADLAR) | ADC_REF_INT_WITH_ECAP;	// встроенный AREF
	ADMUX = _BV(ADLAR) | ADC_REF_AVCC; // особенность схемы "плавающее" смещение
	OCR0 = 250; //!!! для частоты семплов 8000 Гц, т.е. полоса анализа 3750 Гц

	sei();
}

/**
 * Захват семплов ведется
 * с двойной буфферизацией
 * или без нее \see DOUBLE_BUFFERED_SAMPLING
 */
void sample_and_fft(void){
	static uint8_t buf_id;

#if defined(DOUBLE_BUFFERED_SAMPLING)

	while(TCCR0);					// ждем, пока он не остановится

	// меняем буферы местами
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

	TCCR0 = _BV(WGM01) | TIMER_CLK_DIV_8;	// запускаем 0-й таймер

#if !defined(DOUBLE_BUFFERED_SAMPLING)
	while(TCCR0);					// ждем, пока он не остановится
	// обсчитываем только что полученный буфер, пока другой заполняется семплами
	fft32_execute(buffer_fft_a);		// выполняем БПФ над данными
#else
	// обсчитываем только что полученный буфер, пока другой заполняется семплами
	fft32_execute(buf_out);		// выполняем БПФ над данными
#endif
}

/**
 * Возвращает значение указанной гармоники из результатов БПФ
 * @param index порядковый номер гармоники
 * @return относительный уровень гармоники
 */
uint16_t get_result_fft(uint8_t index){
	uint16_t ix;
	__flash const int16_t *revers = tbl32_bitrev;

	revers += index;				// вычисляем указатель на индекс ПЕРЕСТАНОВКИ
	ix = *revers;		// считываем индекс
	return (
#if defined(DOUBLE_BUFFERED_SAMPLING)
	// возвращаем из массива БПФ значение по индексу перестановки
	fsqrt(buf_out[ix].r,buf_out[ix].i)
#else
	fsqrt(buffer_fft_a[ix].r,buffer_fft_a[ix].i)
#endif
	// но перед этим младшие (шумные) биты отбрасываются и амплитуда уменьшается
	// путем сдвига
	 & 0xFFC0)>> 1;
}

/**
 * @}
 */
