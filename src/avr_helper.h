// Вспомогательные макросы для работы с AVR

#ifndef AVR_CONST_H_
#define AVR_CONST_H_

// вспомогательные общие макросы
#define _CONCAT_(x,y)		x ## y
/// вспомогательный макрос слияния символов
#define CONCAT(y,x)		_CONCAT_(y,x)

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/// макрос для определения функции-инициализатора (вызывается автоматически в секции .inix)
#define INIT(x)			static void __attribute__((naked, used, section(".init" #x))) CONCAT(_init_, __COUNTER__) (void)
#define DONE(x)			static void __attribute__((naked, used, section(".fini" #x))) CONCAT(_fini_, __COUNTER__) (void)

#define AUTOINIT()		INIT(2)
/// макрос для определения компактной версии функции main()
#define MAIN()			int __attribute__((OS_main)) main(void)

#define NOINIT			__attribute__((section(".noinit")))

// совместимые типы для целых чисел
typedef uint8_t		U8;
typedef uint16_t	U16;
typedef uint32_t	U32;
typedef int8_t		S8;
typedef int16_t		S16;
typedef int32_t		S32;

// делители тактовой для АЦП
#define ADC_DIV_2	_BV(ADPS0)
#define ADC_DIV_4	_BV(ADPS1)
#define ADC_DIV_8	(_BV(ADBP0) | _BV(ADPS1))
#define ADC_DIV_16	_BV(ADPS2)
#define ADC_DIV_32	(_BV(ADPS0) | _BV(ADPS2))
#define ADC_DIV_64	(_BV(ADPS2) | _BV(ADPS1))
#define ADC_DIV_128	(_BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2))

// события для автозапуска АЦП
/// непрерывный автозапуск
#define ADC_TRIG_FRUN		0
/// аналоговый компаратор
#define ADC_TRIG_ACOMP		_BV(ADTS0)
/// запрос внешнего прерывания INT0
#define ADC_TRIG_INT0		_BV(ADTS1)
/// сравнение канала А нулевого таймера
#define ADC_TRIG_T0_COMPA	(_BV(ADTS0) | _BV(ADTS1))
/// переполнение нулевого таймера
#define ADC_TRIG_T0_OVF		_BV(ADTS2)
/// сравнение канала В первого таймера
#define ADC_TRIG_T1_COMPB	(_BV(ADTS2) | _BV(ADTS0))
/// сравнение канала В нулевого таймера
#define ADC_TRIG_T0_COMPB	ADC_TRIG_T1_COMPB
/// переполнение первого таймера
#define ADC_TRIG_T1_OVF		(_BV(ADTS2) | _BV(ADTS1))
/// изменение состояния пинов
#define ADC_TRIG_PCI		ADC_TRIG_T1_OVF
/// захват первого таймера
#define ADC_TRIG_T1_CAPTURE	(_BV(ADTS2) | _BV(ADTS0) | _BV(ADTS1))

// каналы АЦП
#define ADC_0				0
#define ADC_1				1
#define	ADC_2				2
#define ADC_3				3
#define ADC_4				4
#define ADC_5				5
#define ADC_6				6
#define ADC_7				7

#if defined(__AVR_ATmega88__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega168)
#define ADC_BANDGAP			15
#define ADC_GND				16
#endif

#if defined(__AVR_ATmega128__)
#define ADC_0_DIF_0_G10		8
#define ADC_1_DIF_0_G10		9
#define ADC_0_DIF_0_G200	10
#define ADC_1_DIF_0_G200	11
#define ADC_2_DIF_2_G10		12
#define ADC_3_DIF_2_G10		13
#define ADC_2_DIF_2_G20		14
#define ADC_3_DIF_2_G200	15
#define ADC_0_DIF_1_G1		16
#define	ADC_1_DIF_1_G1		17
#define ADC_2_DIF_1_G1		18
#define ADC_3_DIF_1_G1		19
#define ADC_4_DIF_1_G1		20
#define ADC_5_DIF_1_G1		21
#define ADC_6_DIF_1_G1		22
#define ADC_7_DIF_1_G1		23
#define ADC_0_DIF_2_G1		24
#define ADC_1_DIF_2_G1		25
#define ADC_2_DIF_2_G1		26
#define ADC_3_DIF_2_G1		27
#define ADC_4_DIF_2_G1		28
#define ADC_5_DIF_2_G1		29
#define ADC_BANDGAP			30
#define ADC_GND				31
#endif

#if defined(__AVR_ATtiny26__)
	#define ADC_REF_AVCC			0
	#define ADC_REF_EXTERNAL		_BV(REFS0)
	#define ADC_REF_INT_NO_ECAP		_BV(REFS1)
	#define ADC_REF_INT_WITH_ECAP	(_BV(REFS1) | _BV(REFS0))
#else
	#define ADC_REF_EXTERNAL		0
	#define ADC_REF_AVCC			_BV(REFS0)
	#define ADC_REF_INT_WITH_ECAP	(_BV(REFS1) | _BV(REFS0))
#endif


// аналоговый компаратор - момент возникновения прерываний
#define AC_INT_ON_TOGGLE			0
#define AC_INT_ON_FALLING			_BV(ACIS1)
#define AC_INT_ON_RISING			(_BV(ACIS1) | _BV(ACIS0))

// таймеры
// предделители тактовой
#define TIMER_CLK_DIV_1					1
#define TIMER_CLK_DIV_8					2
#define TIMER_CLK_DIV_64				3
#define TIMER_CLK_DIV_256				4
#define TIMER_CLK_DIV_1024				5
#define TIMER_CLK_EXT_FALL				6
#define TIMER_CLK_EXT_RISE				7
#define TIMER_CLK_DIV(x)				CONCAT(TIMER_CLK_DIV_,x)

// режимы работы выходов OCx - вместо параметра t ввести номер канала таймера,
// например 0 - нулевой таймер, 1А - первый таймер канал А
#define TIMER_OC_NONE(t)			0
#define TIMER_OC_TOGGLE(t)		_BV(COM ## t ## 0)
#define TIMER_OC_CLEAR(t)		_BV(COM ## t ## 1)
#define TIMER_OC_SET(t)			_BV(COM ## t ## 0) | _BV(COM ## t ## 1)

// внешние прерывания - условия срабатывания
// вместо i поставить номер запроса прерывания 0 или 1
#define	EXINT_LOWLEVEL(i)		0
#define EXINT_CHANGE(i)			_BV(ISC ## i ## 0)
#define EXINT_FALLING(i)		_BV(ISC ## i ## 1)
#define EXINT_RISING(i)			_BV(ISC ## i ## 0) | _BV(ISC ## i ## 1)

// делители тактовой SPI
#define SPI_CLK_DIV_4			0
#define SPI_CLK_DIV_16			_BV(SPR0)
#define SPI_CLK_DIV_64			_BV(SPR1)
#define SPI_CLK_DIV_128			_BV(SPR0) | _BV(SPR1)

// модуль TWI
// предделитель тактовой
#define TWI_CLK_DIV_1			0
#define TWI_CLK_DIV_4			_BV(TWPS0)
#define TWI_CLK_DIV_16			_BV(TWPS1)
#define TWI_CLK_DIV_64			_BV(TWPS0) | _BV(TWPS1)

#endif /* AVR_CONST_H_ */
