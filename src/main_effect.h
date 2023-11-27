/** \file main_effect.h
 * \brief Интерфейс визуальных эффектов
 *
 * \author ARV
 * \date	18 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 * \defgroup EFFECTS Визуальные эффекты цветомузыки
 * \brief Программные средства для визуализации мелодии
 *
 * Визуальные эффекты разделены на ОСНОВНЫЕ и ФОНОВЫЕ.
 * \n Основные эффекты сопровождают музыкальный сигнал, т.е. это и есть собственно
 * цветомузыка.
 * \n Фоновые эффекты работают при отсутствии входного сигнала, т.е. реализуют
 * функцию подсветки в тишине.
 * \par Принципы создания эффектов
 * \n Все эффекты реализуются в виде независимых модулей, которые автоматически добавляются
 * в общую систему при помощи функции #register_effect. Модуль эффекта, не добавленный
 * этой функцией, в итоговую прошивку не попадает (исключается оптимизатором компилятора).
 * \n Модуль эффекта должен обязательно содержать главную функцию эффекта, алгоритм
 * работы которой очень прост: необходимо проанализировать полученное в параметре
 * состояние звукового сигнала и на основе этих вычислений обновить содержимое
 * массива пикселов. Главная функция вызывается каждые 10 мс, т.е. 100 раз в секунду,
 * что можно использовать дя отсчета времени при создании эффектов.
 * \n Кроме главной функции, модуль эффекта может содержать и вспомогательные
 * функции, которые используются для изменения алгоритма поведения главной функции.
 * \par Готовые модули:
 * - \ref FGEFFECTS
 * - \ref BGEFFECTS
 *
 * \defgroup FGEFFECTS Основные эффекты
 * Цветомузыкальные эффекты работают, если на вход поступает звуковой сигнал.
 *
 * \defgroup BGEFFECTS Эффекты фоновой подсветки
 *
 * \addtogroup EFFECTS
 * @{
 */

#ifndef MAIN_EFFECT_H_
#define MAIN_EFFECT_H_

#include "global.h"
#include "sd_script.h"

/// предельное количество эффектов любого типа
#define MAX_EFFECT_CNT		7
/// фоновый эффект
#define BACKGROUND_EFFECT	0
/// основной эффект
#define FOREGROUND_EFFECT	1

/// указатель настроку во FLASH
typedef __flash const char * flash_str_t;

/// результат функции смены пресета
typedef enum{
	// любое положительное число или ноль - номер пресета
	PRESET_NOTHING	= -1, ///< нет пресетов
	PRESET_HIDDEN	= -2  ///< отображать название пресета не надо
} preset_result_t;

typedef enum{
	INFO_ONCE,
	INFO_ALWAYS
} effect_info_t;

/// тип структуры описания эффекта
typedef struct {
	flash_str_t		name;					///< название
	void			(*start)(void);			///< начало работы
	void			(*work)(signal_t *s);	///< рендеринг
	preset_result_t	(*preset)(int8_t d);	///< смена пресета
	void			(*stop)(void);			///< конец работы
	void			(*save)(void);			///< сохранение настроек
	effect_info_t	(*info)(uint8_t show);			///< вывод информации
} effect_t;

/// тип описания эффекта, размещенного во FLASH
typedef const __flash effect_t flash_effect_t;

/// Регистрация эффекта в списках
void register_effect(uint8_t toe, flash_effect_t *eff);
/// Рендеринг эффекта
void execute_effect(signal_t *s);
/// Смена пресета для текущегоэффекта последовательно
int8_t change_preset(signal_t *s, int8_t delta);
/// Название текущего эффекта
char* get_effect_name(signal_t *s);
/// Смена эффекта последовательно
void change_effect(signal_t *s, int8_t delta);
/// Выбор эффекта непосредственно
void select_fg_effect(signal_t *s, uint8_t num);
/// Выбор пресета непосредственно
void select_bg_effect(signal_t *s, uint8_t num);
/// Сохранение настроек всех эффектов
void save_all_effects(void);
/// Название текущего пресета для текущего эффекта
//char* get_preset_name(signal_t *s);
void show_info(signal_t *s);

/// Проверка смены фонового эффекта на основной или наоборот
uint8_t effect_auto_changed(signal_t *s);

#endif /* MAIN_EFFECT_H_ */

/**
 * @}
 */
