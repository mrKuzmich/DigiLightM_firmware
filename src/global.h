/** \file global.h
 * \brief Общие описания
 *
 * \author ARV
 * \date	5 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 */

#ifndef EFFECTS_H_
#define EFFECTS_H_
#include <stdint.h>
#include "in_switch.h"

#define MAX_GROUP_OF_PIX		128
#define MIN_GROUP_OF_PIX		16
#define MAX_PIX_IN_GROUP		8
#define MIN_PIX_IN_GROUP		1
#define MAX_TOTAL_PIX			128
#define STEP_GROUP_OF_PIX		8
#define STEP_PIX_IN_GROUP		1

/// Предел уровня громкости, считающийся тишиной
#define DEF_SILENCE_LVL	11

/// наименование гармоник (для килогерцев - округление к большему)
typedef enum harmonic_freq_t{
	F_DC,		///< Постоянная составляющая (инфранизкие частоты)
	F_250_HZ,	///< Гармоника 250 Гц
	F_500_HZ,	///< Гармоника 500 Гц
	F_750_HZ,	///< Гармоника 750 Гц
	F_1000_HZ,	///< Гармоника 1000 Гц
	F_1300_HZ,	///< Гармоника 1250 Гц
	F_1500_HZ,	///< Гармоника 1500 Гц
	F_1800_HZ,	///< Гармоника 1750 Гц
	F_2000_HZ,	///< Гармоника 2000 Гц
	F_2300_HZ,	///< Гармоника 2250 Гц
	F_2500_HZ,	///< Гармоника 2500 Гц
	F_2800_HZ,	///< Гармоника 2750 Гц
	F_3000_HZ,	///< Гармоника 3000 Гц
	F_3300_HZ,	///< Гармоника 3250 Гц
	F_3500_HZ,	///< Гармоника 3500 Гц
	F_3800_HZ,	///< Гармоника 3750 Гц
	// не изменять ниже этого комментария!
	F_CNT		///< Общее количество гармоник
} harmonc_freq_t;

/// обозначения выделенных частотных полос сигнала
typedef enum {
	LF, MF, HF,
	// не изменять ниже этого комментария!
	_BAND_CNT
} band_t;

/// количество выделенных частотных полос сигнала
#define BAND_CNT	_BAND_CNT

/// битовые маски частот спектра, объединенные в НЧ-полосу
#define BAND_LF	_BV(F_250_HZ) |	_BV(F_500_HZ)
/// битовые маски частот спектра, объединенные в СЧ-полосу
#define BAND_MF	_BV(F_750_HZ) | _BV(F_1000_HZ) | _BV(F_1300_HZ) | _BV(F_1500_HZ)
/// битовые маски частот спектра, объединенные в ВЧ-полосу
#define BAND_HF	_BV(F_2800_HZ) | _BV(F_3000_HZ) | _BV(F_3300_HZ) | _BV(F_3500_HZ) | _BV(F_3800_HZ)
/// битовые маски частот спектра, учитываемые при расчете уровня сигнала (все, кроме DC)
#define BAND_FULL ~_BV(F_DC)

/// тип для уровня громкости
typedef uint8_t		volume_t;
/// тип для "мощности" сигналов частотных составляющих
typedef uint16_t	power_t;

/// структура параметров звукового сигнала
typedef struct {
	volume_t	peak_volume;	///< пиковая громкость
	volume_t	average_vol;	///< средняя громкость
	power_t		band[BAND_CNT];	///< "мощности" полос сигнала
	uint8_t		beat;			///< детектор ритма
	uint16_t	harmonics[F_CNT];///< амплитуды всех гармоник сигнала
} signal_t;

/// фиксированное количество уровней регулирования чувствительности
#define MAX_LEVEL	80

/// структура конфигурации устройства
typedef struct {
	uint8_t		clear;				///< признак чистых настроек
	uint8_t		group_of_pixels;	///< количество групп пикселов
	uint8_t		pixels_in_group;	///< количество пикселов в группе
	uint8_t		time_to_sleep;		///< время автоотключения
	uint8_t		agc_enabled;		///< АРУ включено (номер бита соответствует номеру входа)
	uint8_t		lcd_enabled;		///< ЖКИ присутствует
	uint16_t	dc_offset;			/// Настройка постоянной составляющей
	/// TODO: dc_offset логично завести для каждого входа, поскольку по схеме они каждый имеют собственное смещение
	uint8_t		input;				///< номер активного входа
	uint8_t		colog_order;		///< последовательность следования цветовых составляющих
//	uint8_t		mic_gain;			///< усиление микрофона
	// mic_gain будет находится в последнем элементе sensitivity
	uint8_t		sensitivity[IN_CNT];///< уровень регулировки чувствительности (по входам)
	/// На самом деле sensitivity нужен лишь для входов 1, 2 и mic
//	uint16_t	band_mask[BAND_CNT];///< битовые маски полос
} config_t;

extern config_t cfg;

/** флаг блокировки анализа сигнала
 *
 * Если эта переменная не равна нулю, то семплирование сигнала,
 * FFT анализ и вычисление параметров сигнала не выполняется. В этом случае
 * можно задействовать область памяти этих модулей.
 */
extern uint8_t lock_input;
/// запрос резервной памяти
void* get_reserved_memory(uint16_t *size);

typedef uint8_t (*effect_func_t)(signal_t *s);
typedef void (*effect_ini_t)(void);
typedef void (*effect_fini_t)(void);


/// Параметры мелодии
extern signal_t	music; // не хорошо это!


#if defined(_DEBUG_)
#define debug_led_on()			PORT(DEBUG_LED_PORT) &= ~_BV(DEBUG_LED_PIN0)
#define debug_led_off()			PORT(DEBUG_LED_PORT) |= _BV(DEBUG_LED_PIN0)
#define debug_led_toggle()		PORT(DEBUG_LED_PORT) ^= _BV(DEBUG_LED_PIN0)
#else
#define debug_led_on()
#define debug_led_off()
#define debug_led_toggle()
#endif

#include "configuration.h"

#endif /* EFFECTS_H_ */
