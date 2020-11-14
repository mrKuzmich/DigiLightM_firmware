/** \file menu.c
 * \brief система меню
 *
 * Система меню настроек на 2-строчном ЖКИ
 * \author ARV
 * \date	21 мая 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# AVR GCC 5.2.1 или более новая версия
 *
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include "avr_helper.h"
#include <stdlib.h>
#include "setup.h"
#include "global.h"
#include "menu.h"
#include "lcd.h"
#include "lcd_show.h"
#include "control.h"
#include "power_control.h"

__flash const menu_t *current_menu = &main_menu;
static uint16_t menu_timeout = MENU_TIMEOUT;

void reset_menu_timeout(void){
	menu_timeout = MENU_TIMEOUT;
}

uint8_t lock_current_menu_item = 0;

void show_press_encoder_msg(int32_t d){
	show_rpad_str_p(1, PSTR("PUSH ENCODER"));
}

static void paint_menu_item(uint8_t mi){
	int32_t tmp;

	show_rpad_str_p(0, current_menu->items[mi].name);
	switch(current_menu->items[mi].type){
	case MI_SCALE:
		show_scale(1, *(current_menu->items[mi].u8));
		break;
	case MI_ONOFF:
		show_rpad_str_p(1, *(current_menu->items[mi].onoff) == 0 ? PSTR("OFF") : PSTR("ON"));
		break;
	case MI_U8:
		tmp = *(current_menu->items[mi].u8);
		show_number(tmp);
		break;
	case MI_S8:
		tmp = *(current_menu->items[mi].s8);
		show_number(tmp);
		break;
	case MI_U16:
		tmp = *(current_menu->items[mi].u16);
		show_number(tmp);
		break;
	case MI_S16:
		tmp = *(current_menu->items[mi].s16);
		show_number(tmp);
		break;
	case MI_SUBMENU:
		show_press_encoder_msg(0);
		break;
	case MI_USER:
		if(current_menu->items[mi].paint != NULL){
			current_menu->items[mi].paint(current_menu->items[mi].data);
			break;
		}
	default:
		center_str(1, "");
	}
}

static void edit_menu_item(uint8_t mi, int8_t delta){
	int16_t tmp;
	switch(current_menu->items[mi].type){
	case MI_SCALE:
	case MI_U8:
		tmp = *(current_menu->items[mi].u8);
		tmp += delta * current_menu->items[mi].step;
		if(tmp > current_menu->items[mi].u8_max)
			tmp = current_menu->items[mi].u8_max;
		if(tmp < current_menu->items[mi].u8_min)
			tmp = current_menu->items[mi].u8_min;
		*(current_menu->items[mi].u8) = tmp;
		break;
	case MI_S8:
		tmp = *(current_menu->items[mi].s8);
		tmp += delta * current_menu->items[mi].step;
		if(tmp > current_menu->items[mi].s8_max)
			tmp = current_menu->items[mi].s8_max;
		if(tmp < current_menu->items[mi].s8_min)
			tmp = current_menu->items[mi].s8_min;
		*(current_menu->items[mi].s8) = tmp;
		break;
	case MI_U16:
		tmp = *(current_menu->items[mi].u16);
		tmp += delta * current_menu->items[mi].step;
		if(tmp > current_menu->items[mi].u16_max)
			tmp = current_menu->items[mi].u16_max;
		if(tmp < current_menu->items[mi].u16_min)
			tmp = current_menu->items[mi].u16_min;
		*(current_menu->items[mi].u16) = tmp;
		break;
	case MI_S16:
		tmp = *(current_menu->items[mi].s16);
		tmp += delta * current_menu->items[mi].step;
		if(tmp > current_menu->items[mi].s16_max)
			tmp = current_menu->items[mi].s16_max;
		if(tmp < current_menu->items[mi].s16_min)
			tmp = current_menu->items[mi].s16_min;
		*(current_menu->items[mi].s16) = tmp;
		break;
	case MI_ONOFF:
		*(current_menu->items[mi].onoff) = ! *(current_menu->items[mi].onoff);
		break;
	case MI_USER:
		if(current_menu->items[mi].edit != NULL){
			current_menu->items[mi].edit(delta, current_menu->items[mi].data);
		}
		break;
	default:
		break;
	}
}

/**
 *
 * @return 0 - работа с меню завершена, 1 - работа с меню продолжается
 */
menu_result_t do_menu(void){
	static uint8_t menu_item;
	uint8_t handled = 1;
	menu_result_t result = MENU_CONTINUE;

	paint_menu_item(menu_item);

	control_t ctrl = get_control();

	switch(ctrl){
	case CTRL_ENC_BTN:
		// Действие
		if(current_menu->items[menu_item].type == MI_USER){
			if(current_menu->items[menu_item].action != NULL){
				result = current_menu->items[menu_item].action(current_menu->items[menu_item].data);
				if(result == MENU_CONTINUE)
					break;
				else
					goto exit_menu;
			}
		} else if(current_menu->items[menu_item].type == MI_SUBMENU){
			if(current_menu->items[menu_item].submenu != NULL){
				current_menu = current_menu->items[menu_item].submenu;
				menu_item = 0;
			}
			break;
		}
		result = MENU_DONE;
		goto exit_menu;
	case CTRL_ENC_UP:
		edit_menu_item(menu_item, 1);
		break;
	case CTRL_ENC_DN:
		edit_menu_item(menu_item, -1);
		break;
	case CTRL_BTN_SET:
		if(lock_current_menu_item) break;
		menu_item++;
		if(menu_item >= current_menu->item_count) menu_item = 0;
		break;
	default:
		handled = 0;
		if(!--menu_timeout){
			// выход из меню по таймауту
			result = MENU_DONE;
	exit_menu:
			update_config();
			menu_timeout = MENU_TIMEOUT;
			menu_item = 0;
			current_menu = &main_menu;
			reset_power_timeout();
		}
	}

	if(handled) menu_timeout = MENU_TIMEOUT;
	return result;
}
