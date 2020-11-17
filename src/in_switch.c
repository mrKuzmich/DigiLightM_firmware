/** \file in_switch.c
 * \brief Модуль переключателя входов
 *
 * \author DVK
 * \date 19.05.2020
 * \
 * \addtogroup CONTROL
 * @{
 */

#include <avr/io.h>
#include "avr_helper.h"
#include "hardware.h"
#include "in_switch.h"
#include "global.h"

const uint8_t _IN_SWITCH_PORT_MASK = IN_1_SEL_PIN | IN_2_SEL_PIN | IN_MC_SEL_PIN;

static input_t _svitch_state;

void _set_input(void) {
	uint8_t tmp = (PORT(IN_SWITCH_PORT) & ~(_IN_SWITCH_PORT_MASK));
	switch (_svitch_state) {
	case IN_2:
		PORT(IN_SWITCH_PORT) = tmp | IN_2_SEL_PIN;
		break;
	case IN_MIC:
		PORT(IN_SWITCH_PORT) = tmp | IN_MC_SEL_PIN;
		break;
	default: /// при всех прочих - IN_1,
             /// даже если ALL_OFF, при этом включен будет IN_1 и sensitivity будет 0
		PORT(IN_SWITCH_PORT) = tmp | IN_1_SEL_PIN;
	}
}

/// Устанавливает активный вход
void change_input() {
	if (_svitch_state != cfg.input) {
		_svitch_state = cfg.input;
		_set_input();
	}
}

INIT(7) {
	/// пины управления переключателем настраиваем на выход
	DDR(IN_SWITCH_PORT) |= _IN_SWITCH_PORT_MASK;
	_svitch_state = cfg.input;
	_set_input();
}


