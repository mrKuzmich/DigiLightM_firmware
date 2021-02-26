/** \file menu.h
 * \brief ������� ����
 *
 * �������� ������� ���� ��� ������������ ��� � ���������� ��������������� ����������
 * \author ARV
 * \date	21 ��� 2017 �.
 * \copyright Copyright 2015 � ARV. All rights reserved.
 *
 * ��� ���������� ���������:\n
 * 	-# AVR GCC 5.2.1 ��� ����� ����� ������
 *
 */

#ifndef MENU_H_
#define MENU_H_

extern __flash const char VOL_NAME[];
extern __flash const char STR_SOURCE[];
extern __flash const char *in[];

extern uint8_t lock_current_menu_item;

/// �������� ������� ����
typedef enum{
	MI_SUBMENU,	///< �������
	MI_ONOFF,	///< ������
	MI_SCALE,	///< �����
	MI_U8,		///< ���� ��� �����
	MI_U16,		///< int ��� �����
	MI_S8,		///< ���� �� ������
	MI_S16,		///< int
	MI_USER		///< ���������������� �����
} item_t;

/// ������� ��������� ����
typedef enum{
	MENU_DONE,		///< ������ � ���� ���������
	MENU_DONE_MSG,	///< ������ � ���� ��������� � ������� ���������
	MENU_CONTINUE	///< ������ ������������
} menu_result_t;

/// ����������� ��� ������ ����
typedef struct{
	__flash const char *name;	///< ������ - �������� ������
	item_t				type;	///< ������ - ������� ������
	union{
		// MI_SUBMENU
		__flash const void *submenu;///< ��� ������� - ������ �� �������
		// MI_ONOFF
		uint8_t			*onoff;		///< ��� ������ - ������ �� ����
		// MI_SCALE, MI_U8
		struct{
			uint8_t 	step;		///< ��� ����� � ����� - ��� ��������� ��� ��������������
			union{
				struct{
					uint8_t		*u8;	///< ��� ����� ��� ����� - ������ �� ����������
					uint8_t		u8_min;	///< ��� ����� ��� ����� - ���������� �������
					uint8_t		u8_max; ///< ��� ����� ��� ����� - ���������� ��������
				};
				// MI_U16
				struct{
					uint16_t	*u16;	///< ��� int ��� ����� - ������ �� ����������
					uint16_t	u16_min;///< ��� int ��� ����� - ���������� �������
					uint16_t	u16_max;///< ��� int ��� ����� - ���������� ��������
				};
				// MI_S8
				struct{
					int8_t		*s8;	///< ��� ����� - ������ �� ����������
					int8_t		s8_min;	///< ��� ����� - ���������� �������
					int8_t		s8_max;	///< ��� ����� - ���������� ��������
				};
				// MI_S16
				struct{
					int16_t		*s16;	///< ��� int - ������ �� ����������
					int16_t		s16_min;///< ��� int - ���������� �������
					int16_t		s16_max;///< ��� int - ���������� ��������
				};
			};
		};
		// MI_USER
		struct{
			void			(*edit)(int8_t d, uint16_t data);///< ��� ����������������� - ��������, ��� �����
			void			(*paint)(int32_t d);	///< ��� ����������������� - ������� 2-� ������
			menu_result_t	(*action)(uint16_t d);///< ��� ����������������� - ������ 0, ���� ���� ��������� ����
			uint16_t		data; ///< ��� ����������������� - �������������� ������
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

/// ����
typedef struct{
	uint8_t item_count;
	__flash const menu_item_t __flash const *items;
} menu_t;

/// ������� ��� ���������� �� ���� � 10��-��������
#define MENU_TIMEOUT	1500

extern __flash const menu_t main_menu;

/// ����� � ��������� ���� ��������
menu_result_t do_menu(void);

void reset_menu_timeout(void);

void show_press_encoder_msg(int32_t d);

extern __flash const menu_t *current_menu;

#endif /* MENU_H_ */
