/** \file setup.c
 * \brief Модуль настроек цветомузыки
 *
 * \author ARV
 * \date	19 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <avr/eeprom.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "power_control.h"
#include "global.h"
#include "lcd.h"
#include "lcd_show.h"
#include "hardware.h"
#include "setup.h"
#include "control.h"
#include "ir_remote.h"
#include "menu.h"
#include "in_switch.h"
#include "gain_ctrl.h"
#include "ws2812.h"

/**
 * \defgroup MENU Система меню
 * Меню настроек устройства
 *
 * \addtogroup MENU
 * @{
 *
 * Вход и переключение пунктов - отдельная кнопка MENU
 * Изменение значения в пункте - вращением энкодера
 * Принятие нового значения (и выход из меню) - кнопка энкодера.
 * Отмена и выход из меню - ничегонеделание в течение 15 секунд.
 * Каждый пункт выводится в обеих строках ЖКИ: первая строка название пункта,
 * вторая строка - значение (с вспомогательным текстом, если необходимо).
 *
 * Во время работы с меню основные функции продолжают выполняться, т.е. все
 * световые эффекты работают.
 */

__flash const char VOL_NAME[]	= "SENSITIVITY";
static __flash const char str1[]		= "ACTIVE INPUT";
static __flash const char str2[]		= "A.G.C.";
static __flash const char str3[]		= "SLEEP TIME";
static __flash const char str4[]		= "LEDS IN GROUP";
static __flash const char str5[]		= "GROUPS OF LEDS";
static __flash const char str6[]		= "RESET TO DEFAULT";
static __flash const char str7[]		= "IR REMOTE MENU";
static __flash const char str8[]		= "DC OFFSET";
static __flash const char str9[]		= "MICROPHONE GAIN";
static __flash const char strA[]		= "COLORS ORDER";

static __flash const char ir0[]			= "POWER OFF";
static __flash const char ir1[]			= "VOLUME+";
static __flash const char ir2[]			= "VOLUME-";
static __flash const char ir3[]			= "EFFECT NEXT";
static __flash const char ir4[]			= "EFFECT PREV";
static __flash const char ir5[]			= "PRESET NEXT";
static __flash const char ir6[]			= "PRESET PREV";
static __flash const char ir7[]			= "DONE UP LEARNING";

static __flash const char in0[]			= "ALL OFF";
static __flash const char in1[]			= "INPUT 1";
static __flash const char in2[]			= "INPUT 2";
static __flash const char in3[]			= "MICROPHONE";

static __flash const char* in[]			= {in0, in1, in2, in3};

static __flash const char m_gain0[]		= "40dB";
static __flash const char m_gain1[]		= "50dB";
static __flash const char m_gain2[]		= "60dB";

static __flash const char* m_gain[]		= {m_gain0, m_gain1, m_gain2};


static void default_setup(void){
	cfg.pixels_in_group = 1;
	cfg.group_of_pixels = 96;
	cfg.time_to_sleep = 0;
	for (uint8_t i = 0; i < IN_CNT; i++)
		cfg.sensitivity[i] = i == IN_OFF ? 0 : // для отключенных входов
								i == IN_MIC ? 1 : // для микрофонного входа - 40db
										50; // для обычных входов
	cfg.agc_enabled = 0;
	cfg.input = 1;
}

static void default_ir_setup(void){
#include "ir_car_mp3.h"
	init_ir_cmd_code(0, CMD_VOLUME_PLUS, IR_VOL_PLUS);
	init_ir_cmd_code(1, CMD_VOLUME_MINUS, IR_VOL_MINUS);
	init_ir_cmd_code(2, CMD_EFFECT_PLUS, IR_CH_PLUS);
	init_ir_cmd_code(3, CMD_EFFECT_MINUS, IR_CH_MINUS);
	init_ir_cmd_code(4, CMD_PRESET_PLUS, IR_NEXT);
	init_ir_cmd_code(5, CMD_PRESET_MINUS, IR_PREV);
	init_ir_cmd_code(6, CMD_POWER, IR_NO_CMD);
}

static EEMEM config_t e_cfg = {
	.clear = 0xFF /*,
	.group_of_pixels = 96,
	.pixels_in_group = 1,
	.time_to_sleep = 10,
	.agc_enabled = 0,
	.lcd_enabled = 1,
	.dc_offset = 0x7FE0,
	.input = 1,
	.color_order = 0,
	.sensitivity = {0, 50, 50, 1}*/
};

void update_config(void){
	eeprom_update_block(&cfg, &e_cfg, sizeof(config_t));
}

INIT(6){
	// этот модуль обязан инициализировать раньше, чем lcd_show !
	eeprom_read_block(&cfg, &e_cfg, sizeof(config_t));
	if(cfg.clear){
//	if(1){
		memset(&cfg, 0, sizeof(config_t));
		default_setup();
/*
		cfg.band_mask[0] = BAND_LF;
		cfg.band_mask[1] = BAND_MF;
		cfg.band_mask[0] = BAND_HF; // TODO ???
*/
		cfg.dc_offset = 0x7FE0;
		eeprom_update_block(&cfg, &e_cfg, sizeof(config_t));
		default_ir_setup();
	}
}

static menu_result_t reset_to_default(uint16_t d){
	default_setup();
	update_config();
	lcd_clrscr();
	lcd_puts_P("ALL SETTINGS\nRESET TO DEFAULT");
	_delay_ms(1000);
	return MENU_DONE;
}

#define DC_OFS_MIN	29696UL
#define DC_OFS_MAX	35200UL

static void edit_dc(int8_t d, uint16_t data){
	uint16_t tmp = cfg.dc_offset;
	tmp += d*0x40;
	if(tmp < DC_OFS_MIN) tmp = DC_OFS_MIN;
	else if(tmp > DC_OFS_MAX) tmp = DC_OFS_MAX;
	cfg.dc_offset = tmp;
}

static void paint_dc(int32_t d){
	char tmp[10];
	signal_t *ptr = (void*)(int)d;

	lcd_gotoxy(0,1);
	lcd_puts_P("DC=");
	itoa(ptr->average_vol, tmp, 10);
	lcd_puts(tmp);
	lcd_puts_P("/");
	utoa(cfg.dc_offset, tmp, 10);
	lcd_puts(tmp);
	lcd_puts_P("         ");
}

#define ir_wait lock_current_menu_item
static uint32_t ir_code;

static void ir_paint(int32_t d){
	static uint8_t div;

	uint32_t tmp;
	tmp = ir_wait ? get_ir_code() : get_ir_cmd_code(d);
	if(tmp) ir_code = tmp;

	lcd_gotoxy(0,1);
	lcd_puts_P("CODE: ");

	if((++div & 0x20) && ir_wait)
		lcd_puts_P("          ");
	else
		if((ir_code == 0) || (ir_code == IR_NO_CMD)){
			lcd_puts_P("--------  ");
		} else {
			show_hex_long(ir_code);
		}

	if(ir_wait) reset_menu_timeout();
}

static menu_result_t ir_action(uint16_t d){
	if(ir_wait){
		set_ir_cmd_code(d, ir_code);
		ir_wait = 0;
	} else {
		ir_wait = 1;
		ir_code = 0;
		set_ir_cmd_code(d, IR_NO_CMD);
	}
	return MENU_CONTINUE;
}

static menu_result_t ir_action_reset(uint16_t d){
	default_ir_setup();
	lcd_clrscr();
	lcd_puts_P("IR-REMOTE RESET\nTO DEFAULT");
	return MENU_DONE_MSG;
}

static void edit_pix(int8_t d, uint16_t data){
	// data - указатель на поле в cfg
	int16_t tmp = *(uint8_t *)data;
	uint8_t gop = cfg.group_of_pixels;
	uint8_t pig = cfg.pixels_in_group;

	if((uint8_t *)data == &cfg.group_of_pixels){
		// кол-во групп
		tmp += d*STEP_GROUP_OF_PIX;
		if(tmp < MIN_GROUP_OF_PIX) tmp = MIN_GROUP_OF_PIX;
		if(tmp > MAX_GROUP_OF_PIX) tmp = MAX_GROUP_OF_PIX;
	} else {
		// пикселов в группе
		tmp += d*STEP_PIX_IN_GROUP;
		if(tmp < MIN_PIX_IN_GROUP) tmp = MIN_PIX_IN_GROUP;
		if(tmp > MAX_PIX_IN_GROUP) tmp = MAX_PIX_IN_GROUP;
	}
	*(uint8_t *)data = tmp;
	if((cfg.group_of_pixels * cfg.pixels_in_group) > MAX_TOTAL_PIX){
		cfg.group_of_pixels = gop;
		cfg.pixels_in_group = pig;
	}
}

static void paint_pix(int32_t d){
	show_number(*(uint8_t*)(uint16_t)d);
}

// Меню переключения входа
static void edit_input(int8_t d, uint16_t data) {
	uint8_t value = *(uint8_t *)data;
	value += d;
	if (value == UINT8_MAX) value = IN_MIC;
	if (value == IN_CNT) value = IN_OFF;
	*(uint8_t *)data = value;
	change_input();
}

static void paint_input(int32_t d) {
	center_str_p(1, in[*(uint8_t*)(uint16_t)d]);
}

// Меню регулировки чувствительности
static void edit_sensitivity(int8_t d, uint16_t data) {
    if(cfg.input != IN_OFF && cfg.input != IN_MIC) {
        cfg.sensitivity[cfg.input] += d;
        if (cfg.sensitivity[cfg.input] <= 0)
            cfg.sensitivity[cfg.input] = 0;
        if (cfg.sensitivity[cfg.input] >= MAX_LEVEL)
            cfg.sensitivity[cfg.input] = MAX_LEVEL;
    }
}

static void paint_sensitivity(int32_t d) {
    if(cfg.input == IN_OFF || cfg.input == IN_MIC)
        center_str_p(1, in[cfg.input]);
    else
        show_scale(1, cfg.sensitivity[cfg.input]);
}

// Меню регулировки чувствительности микрофона
static void edit_mgain(int8_t d, uint16_t data) {
    change_val((int8_t *)data, _MG_COUNT, d);
    change_mgain();
}

static void paint_mgain(int32_t d) {
	center_str_p(1, m_gain[*(uint8_t*)(uint16_t)d]);
}

// Настройка порядка следования цветов
static void edit_order(int8_t d, uint16_t data) {
    change_val((int8_t *)data, COLOR_SEQ_COUNT, d);
}

static void paint_order(int32_t d) {
    lcd_gotoxy(0,1);
    for(uint8_t c_s = color_sequence[cfg.color_order]; c_s; c_s >>= 2) {
        switch (c_s & COLOR_MASK) {
            case C_R:
                lcd_puts_P("R");
                break;
            case C_G:
                lcd_puts_P("G");
                break;
            case C_B:
                lcd_puts_P("B");
        }
    }
}


static menu_result_t update_and_reboot(uint16_t d){
	update_config();
	lcd_clrscr();
	lcd_puts_P("ALL SETTINGS\nSAVED");
	_delay_ms(1000);
	return MENU_DONE;
}

static __flash const menu_item_t __flash const ir_menu_items[] = {
	_MI_USER(ir0, NULL, ir_paint, ir_action, CMD_POWER),
	_MI_USER(ir1, NULL, ir_paint, ir_action, CMD_VOLUME_PLUS),
	_MI_USER(ir2, NULL, ir_paint, ir_action, CMD_VOLUME_MINUS),
	_MI_USER(ir3, NULL, ir_paint, ir_action, CMD_EFFECT_PLUS),
	_MI_USER(ir4, NULL, ir_paint, ir_action, CMD_EFFECT_MINUS),
	_MI_USER(ir5, NULL, ir_paint, ir_action, CMD_PRESET_PLUS),
	_MI_USER(ir6, NULL, ir_paint, ir_action, CMD_PRESET_MINUS),
	_MI_USER(ir7, NULL, show_press_encoder_msg, NULL, 0),
	_MI_USER(str6, NULL, show_press_encoder_msg, ir_action_reset, 0)
};

static __flash const menu_t ir_menu = _MENU(ir_menu_items);

static __flash const menu_item_t __flash const main_menu_items[] = {
//	_MI_SCALE(VOL_NAME, cfg.sensitivity[IN_1], 0, MAX_LEVEL, 1),
    _MI_USER(VOL_NAME, edit_sensitivity, paint_sensitivity, update_and_reboot, &cfg.input),
	_MI_USER(str1, edit_input, paint_input, update_and_reboot, &cfg.input),
	_MI_USER(str9, edit_mgain, paint_mgain, update_and_reboot, &cfg.sensitivity[IN_MIC]),
	_MI_ONOFF(str2, cfg.agc_enabled),
    _MI_U8(str3, cfg.time_to_sleep, 0, 60, 5),
	_MI_USER(str5, edit_pix, paint_pix, update_and_reboot, &cfg.group_of_pixels),
	_MI_USER(str4, edit_pix, paint_pix, update_and_reboot, &cfg.pixels_in_group),
    _MI_USER(strA, edit_order, paint_order, update_and_reboot, &cfg.color_order),
	_MI_USER(str8, edit_dc, paint_dc, update_and_reboot, &music),
	_MI_SUBMENU(str7, ir_menu),
	_MI_USER(str6, NULL, show_press_encoder_msg, reset_to_default,0)
};

__flash const menu_t main_menu = _MENU(main_menu_items);

/**
 * @}
 */
