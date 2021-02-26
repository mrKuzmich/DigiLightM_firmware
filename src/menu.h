/** \file menu.h
 * \brief система меню
 *
 * Описание системы меню для двустрочного ЖКИ с встроенным редактированием параметров
 * \author ARV
 * \date	21 мая 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#ifndef MENU_H_
#define MENU_H_

extern __flash const char VOL_NAME[];
extern __flash const char STR_SOURCE[];
extern __flash const char *in[];

extern uint8_t lock_current_menu_item;

/// варианты пунктов меню
typedef enum{
	MI_SUBMENU,	///< субменю
	MI_ONOFF,	///< флажок
	MI_SCALE,	///< шкала
	MI_U8,		///< байт без знака
	MI_U16,		///< int без знака
	MI_S8,		///< байт со знаком
	MI_S16,		///< int
	MI_USER		///< пользовательский пункт
} item_t;

/// автомат состояния меню
typedef enum{
	MENU_DONE,		///< работа с меню завершена
	MENU_DONE_MSG,	///< работа с меню завершена с выдачей сообщения
	MENU_CONTINUE	///< работа продолжается
} menu_result_t;

/// вариативный тип пункта меню
typedef struct{
	__flash const char *name;	///< всегда - название пункта
	item_t				type;	///< всегда - вариант пункта
	union{
		// MI_SUBMENU
		__flash const void *submenu;///< для субменю - ссылка на субменю
		// MI_ONOFF
		uint8_t			*onoff;		///< для флажка - ссылка на флаг
		// MI_SCALE, MI_U8
		struct{
			uint8_t 	step;		///< для чисел и шкалы - шаг изменения при редактировании
			union{
				struct{
					uint8_t		*u8;	///< для байта без знака - ссылка на переменную
					uint8_t		u8_min;	///< для байта без знака - допустимый минимум
					uint8_t		u8_max; ///< для байта без знака - допустимый максимум
				};
				// MI_U16
				struct{
					uint16_t	*u16;	///< для int без знака - ссылка на переменную
					uint16_t	u16_min;///< для int без знака - допустимый минимум
					uint16_t	u16_max;///< для int без знака - допустимый максимум
				};
				// MI_S8
				struct{
					int8_t		*s8;	///< для байта - ссылка на переменную
					int8_t		s8_min;	///< для байта - допустимый минимум
					int8_t		s8_max;	///< для байта - допустимый максимум
				};
				// MI_S16
				struct{
					int16_t		*s16;	///< для int - ссылка на переменную
					int16_t		s16_min;///< для int - допустимый минимум
					int16_t		s16_max;///< для int - допустимый максимум
				};
			};
		};
		// MI_USER
		struct{
			void			(*edit)(int8_t d, uint16_t data);///< для пользовательского - изменяет, что хочет
			void			(*paint)(int32_t d);	///< для пользовательского - выводит 2-ю строку
			menu_result_t	(*action)(uint16_t d);///< для пользовательского - вернет 0, если надо закончить меню
			uint16_t		data; ///< для пользовательского - дополнительные данные
		};
	};
} menu_item_t;

#define _MI_SUBMENU(n,_sm)	{.name = n, .type = MI_SUBMENU, .submenu = &_sm}
#define _MI_ONOFF(n,_onoff)	{.name = n, .type = MI_ONOFF, .onoff = &_onoff}
#define _MI_SCALE(n,v,_min,_max,_step)	{.name = n, .type = MI_SCALE, .u8 = &v, .u8_min = (_min), .u8_max = (_max), .step = (_step)}
#define _MI_U8(n,v,_min,_max,_step)	{.name = n, .type = MI_U8, .u8 = &v, .u8_min = (_min), .u8_max = (_max), .step = (_step)}
#define _MI_S8(n,v,_min,_max,_step)	{.name = n, .type = MI_S8, .s8 = &v, .s8_min = (_min), .s8_max = (_max), .step = (_step)}
#define _MI_U16(n,v,_min,_max,_step)	{.name = n, .type = MI_U16, .u16 = &v, .u16_min = (_min), .u16_max = (_max), .step = (_step)}
#define _MI_S16(n,v,_min,_max,_step)	{.name = n, .type = MI_S16, .s16 = &v, .s16_min = (_min), .s16_max = (_max), .step = (_step)}
#define _MI_USER(n,_edit,_paint,_action,_data)	{.name = n, .type = MI_USER, .edit = _edit, .paint = _paint, .action = _action, .data = (uint16_t)(_data)}

#define _MENU(mi) {.item_count = sizeof(mi) / sizeof(menu_item_t), .items = mi}
#define FLASH_MENU(id,mi) __flash const menu_t id = _MENU(mi)
#define RAM_MENU(id,mi) const menu_t id = _MENU(mi)

/// меню
typedef struct{
	uint8_t item_count;
	__flash const menu_item_t __flash const *items;
} menu_t;

/// интевал для автовыхода из меню в 10мс-отрезках
#define MENU_TIMEOUT	1500

extern __flash const menu_t main_menu;

/// вывод и обработка меню настроек
menu_result_t do_menu(void);

void reset_menu_timeout(void);

void show_press_encoder_msg(int32_t d);

extern __flash const menu_t *current_menu;

#endif /* MENU_H_ */
