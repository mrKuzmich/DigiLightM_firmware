/** \file control.c
 * \brief Управление и индикация
 * Модуль, обеспечивающий обработку вращения и нажатий энкодера и кнопок, а
 * так же выполняющий вывод информации на ЖКИ.

 * \author ARV
 * \date	9 апр. 2017 г.
 * \copyright Copyright 2015 © ARV. All rights reserved.
 *
 * \addtogroup CONTROL
 */

#include <avr/io.h>
#include "avr_helper.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "power_control.h"
#include <string.h>
#include <stdlib.h>
#include "gain_ctrl.h"
#include "encoder.h"
#include "lcd.h"

#include "pixel.h"
#include "control.h"
#include "global.h"
#include "main_effect.h"
#include "lcd_show.h"
#include "ir_remote.h"
#include "menu.h"
#include "setup.h"

#if defined(_FFT_DEBUG_)
#include "fft_debug.h"
#endif

/**
 * Изменение однобайтной переменной с ограничением значений.
 * @param v изменяемая переменная
 * @param lim непересекаемая граница значения
 * @param d шаг изменения значения
 */
static void change_val(int8_t *v, uint8_t lim, int8_t d){
	int16_t tmp = *v;
	tmp += d;
	if(tmp >= lim) tmp = lim-1;
	if(tmp < 0) tmp = 0;
	*v = tmp;
}

/// Флаги автомата состояния опроса кнопок
enum button_state{
	BTN_WAIT_RELEASED,	///< Ожидание отпускания кнопки
	BTN_WAIT_PRESSED,	///< Ожидание нажатия кнопки
	BTN_WAIT_DEBOUNCE	///< Подавление дребезга
};

/// Вспомогательный тип для автомата состояний опроса кнопок
typedef struct{
	uint8_t state : 2;	///< Состояние автомата
	uint8_t div	  : 1;	///< Флаг повторного входа вопрос
} btn_state_t;

/**
 * Регулировка уровня чувствительности
 * @param delta направление изменения уровня
 */
static void change_level(int8_t delta){
	if(cfg.agc_enabled)
		return; // при включении АРУ ручное регулирование не работает
	int8_t lvl = cfg.sensitivity[cfg.input];
	change_val(&lvl, MAX_LEVEL, delta);
	cfg.sensitivity[cfg.input] = lvl;
}

/**
 * Состояние кнопок
 * @return битовые значения нажатых кнопок
 */
uint8_t get_btn_state(void){
	return ~PIN(BTN_PORT) & (BTN_CHANGE | BTN_MENU);
}

// признак нахождения внутри системы меню, т.е. индикация "не обычная"
static uint8_t in_menu;

/**
 * получение кода нажатой кнопки
 * \note Подавление дребезга осуществляется при помощи автомата состояний,
 * т.е. функция должна быть вызвана минимум дважды для обнаружения нажатия.
 * @return код кнопки
 */
static uint8_t get_pressed_btn(void){
	static btn_state_t st = {.state = BTN_WAIT_RELEASED};
	static uint8_t old;
	uint8_t btn = get_btn_state();

	switch(st.state){
	case BTN_WAIT_RELEASED:
		if(!btn){
			st.state = BTN_WAIT_PRESSED;
			st.div = 1;
			old = 0;
		}
		break;
	case BTN_WAIT_PRESSED:
		if(btn){
			old = btn;
			st.div = 1;
			st.state = BTN_WAIT_DEBOUNCE;
		}
		break;
	case BTN_WAIT_DEBOUNCE:
		if(st.div){
			st.div = 0;
			break;
		}
		if(old == btn){
			st.state = BTN_WAIT_RELEASED;
			return btn;
		}
	}
	return BTN_NONE;
}

/**
 * получение команды управления от физических органов управления
 * TODO реализовать команду выключения питания при длительном нажатии кнопки
 * (энкодера или меню?)
 * @return
 */
control_t get_control(void){
	// сначала кнопки
	uint8_t btn = get_pressed_btn();
	if(btn != BTN_NONE){
		switch(btn){
		case BTN_CHANGE:
			return CTRL_ENC_BTN;
		case BTN_MENU:
			return CTRL_BTN_SET;
		}
	}
	// затем энкодер
	int8_t enc = get_encoder();
	if(enc != 0){
		if(enc > 0)
			return btn == BTN_CHANGE ? CTRL_ENC_BTN_UP : CTRL_ENC_UP;
		else
			return btn == BTN_CHANGE ? CTRL_ENC_BTN_DN : CTRL_ENC_DN;
	} else {
		if(btn == BTN_CHANGE) return CTRL_BTN_SET;
	}

	return CTRL_NONE;
}

/**
 * получение команды управления от всех органов управления, включая пульт и др.
 * @param ctrl код от физических органов управления
 * @return команда управления
 * \note физические органы управления имеют приоритет над "внешними"
 */
static control_cmd_t get_command(control_t ctrl){
	// сначала обработка физических органов управления
	switch(ctrl){
	case CTRL_BTN_PWR:
		return CMD_POWER;
	case CTRL_BTN_SET:
		return CMD_SETUP;
	case CTRL_ENC_BTN_UP:
	case CTRL_ENC_BTN:
		return CMD_EFFECT_PLUS;
	case CTRL_ENC_BTN_DN:
		return CMD_EFFECT_MINUS;
	case CTRL_ENC_UP:
		return CMD_PRESET_PLUS;
	case CTRL_ENC_DN:
		return CMD_PRESET_MINUS;
	default:
		break;
	}
	// TODO добавить ввод команд из UART
	// а затем - пульт IR
	return get_ir_command();
}

// вспомогательный тип режима индикации
typedef enum{
	EXEC_NO_SHOW,	// ничего не обновлять на ЖКИ
	EXEC_SHOW,		// обновить ЖКИ в текущем режиме
	EXEC_SCALE		// показать шкалу чувствительности на ЖКИ
} exec_result_t;

/**
 * исполнение команды управления
 * @param s параметры звукового сигнала
 * @return способ обновления индикации
 */
static exec_result_t exec_cmd(signal_t *s){
	control_cmd_t cmd = get_command(get_control());
	exec_result_t result = EXEC_SHOW;

	switch(cmd){
	case CMD_POWER:
		power_off();
		break;
	case CMD_VOLUME_PLUS:
		change_level(1);
		result = EXEC_SCALE;
		break;
	case CMD_VOLUME_MINUS:
		change_level(-1);
		result = EXEC_SCALE;
		break;
	case CMD_EFFECT_PLUS:
		change_effect(s, 1);
		break;
	case CMD_EFFECT_MINUS:
		change_effect(s, -1);
#if !defined(ONLY_BASE_CMD)
	case CMD_EFFECT_1 ... CMD_EFFECT_10:
		// TODO выбор конкретного эффекта
		//select_fg_effect(s, cmd - CMD_EFFECT_1);
		//select_bg_effect(s, cmd - CMD_PRESET_1);
		break;
#endif
	case CMD_PRESET_PLUS:
		change_preset(s, 1);
		break;
	case CMD_PRESET_MINUS:
		change_preset(s, -1);
		break;
#if !defined(ONLY_BASE_CMD)
	case CMD_PRESET_1 ... CMD_PRESET_10:
		// TODO выбор конкретного пресета
		break;
#endif
	case CMD_SETUP:
		in_menu = 1;
		break;
	default:
		result = EXEC_NO_SHOW;
	}

	return result;
}

/**
 * Обрабатывает нажатия кнопок и выполняет соответствующие действия,
 * обновляет ЖКИ, если он есть, выполняет функции АРУ и т.п.
 * @param s параметры звукового сигнала
 */
void do_control(signal_t *s){
	static uint8_t timeout;
	uint8_t do_show = 0;
	exec_result_t exer;

	if(in_menu && cfg.lcd_enabled){
		// отрабатываем систему меню
		switch(do_menu()){
		case MENU_CONTINUE:
			// продолжение меню
			return;
		case MENU_DONE:
			// выход из меню
			in_menu = 0;
			do_show = 1;
			break;
		case MENU_DONE_MSG:
			// выход из меню с сообщением
			in_menu = 0;
			timeout = IND_TIMEOUT;
			break;
		}
	}

	// получаем и обрабатываем команду
	exer = exec_cmd(s);
	if(!do_show)
		do_show = exer != EXEC_NO_SHOW;
	// если ЖКИ будет обновляться, сбрасываем таймаут автовыключения питания
	if(do_show)	reset_power_timeout();

#if !defined(_FFT_DEBUG_)
	do_show = do_show || effect_auto_changed(s);

	if(cfg.lcd_enabled){
		if(do_show || !timeout){
			if(exer == EXEC_SCALE){
				show_rpad_str_p(0, VOL_NAME);
				show_scale(1, cfg.sensitivity[cfg.input]);
			} else {
				center_str(0, get_effect_name(s));
				//preset_name = get_preset_name(s);
				if(do_show)	//center_str(1, preset_name);
					show_info(s);
				else
					show_spectrum(s);
			}
			timeout = (do_show) ? IND_TIMEOUT : 0;
		}
		if(timeout) timeout--;
	}
#else
	_fft_debug_show(s);
#endif
}

/// Инициализация модуля
INIT(7){
	// кнопки
	DDR(BTN_PORT) &= ~BTN_ANY;
	PORT(BTN_PORT) |= BTN_ANY;
}

/**
 * @}
 */
